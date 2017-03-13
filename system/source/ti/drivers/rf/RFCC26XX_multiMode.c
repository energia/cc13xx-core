/*
* Copyright (c) 2015-2017, Texas Instruments Incorporated
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
* *  Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
* *  Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
* *  Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
* THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
* PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
* EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <xdc/runtime/Assert.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/family/arm/m3/Hwi.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Swi.h>

#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26XX.h>
#include <ti/drivers/rf/RF.h>

#ifdef DEVICE_FAMILY
    #undef DEVICE_FAMILY_PATH
    #define DEVICE_FAMILY_PATH(x) <ti/devices/DEVICE_FAMILY/x>
#else
    #error "You must define DEVICE_FAMILY at the project level as one of cc26x0, cc26x0r2, cc13x0, etc."
#endif

#include DEVICE_FAMILY_PATH(inc/hw_memmap.h)
#include DEVICE_FAMILY_PATH(inc/hw_ints.h)
#include DEVICE_FAMILY_PATH(inc/hw_types.h)
#include DEVICE_FAMILY_PATH(inc/hw_rfc_rat.h)
#include DEVICE_FAMILY_PATH(inc/hw_rfc_dbell.h)
#include DEVICE_FAMILY_PATH(driverlib/rfc.h)
#include DEVICE_FAMILY_PATH(driverlib/sys_ctrl.h)
#include DEVICE_FAMILY_PATH(driverlib/ioc.h)
#include DEVICE_FAMILY_PATH(driverlib/aon_ioc.h)
#include DEVICE_FAMILY_PATH(driverlib/rf_mailbox.h)
#include DEVICE_FAMILY_PATH(driverlib/adi.h)
#include DEVICE_FAMILY_PATH(driverlib/aon_rtc.h)
#include DEVICE_FAMILY_PATH(driverlib/chipinfo.h)


#if defined(__IAR_SYSTEMS_ICC__)
#pragma diag_remark=Pa082
#endif

/*-------------- Typedefs, structures & defines ---------------*/
typedef enum {
    Fsm_EventBootDone           = (1<<30),
    Fsm_EventPowerStep          = (1<<29),
    Fsm_EventLastCommandDone    = (1<<1),
    Fsm_EventWakeup             = (1<<2),
    Fsm_EventPowerDown          = (1<<3),
    Fsm_EventInitChangePhy      = (1<<10),
    Fsm_EventChangePhy1         = (1<<11),
    Fsm_EventChangePhy2         = (1<<12),
    Fsm_EventHwInt              = (1<<13),
    Fsm_EventCpeInt             = (1<<14),
    Fsm_EventIeeeInt            = (1<<15),
} Fsm_Event;

typedef enum
{
    RF_PowerConstraintCmdQ      = (1 << 0),  // Power constraint from command queue
    RF_PowerConstraintRat       = (1 << 1),  // Power constraint from RAT
    RF_PowerConstraintAll       = (1 << 7)   // Power constraint to disable power management
} RF_PowerConstraintSrc;

typedef enum
{
    RF_ScheduleCmdSuccess       = 0,         // Schedule command success
    RF_ScheduleCmdAllocError    = 1,         // Schedule command allocation error (such as queue full)
    RF_ScheduleCmdSchError      = 2          // SChedule command scheduler error (timing or priority conflict)
}RF_ScheduleCmdStatus;

/* Max # of RF driver clients */
#define N_MAX_CLIENTS               2
/* 8 RF_Cmds in pool */
#define N_CMD_POOL                  8
/* Modulus mask used for RF_CmdHandle calculations */
#define N_CMD_MODMASK               0xFFF

/* Macros */
#define ABS(x)   (((x) < 0) ? -(x) : (x))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))

/* Internal RF constants */
#define RF_CMD0                                0x0607
/* Accessible RF Core interrupts mask MSB 32 bits : RFHW int, LSB 32 bits : RF CPE int */
#define RF_INTERNAL_IFG_MASK                   0xFFFFFFDF60001002
#define RF_CMD_ABORTED_FLAG                    (1 << 0)
#define RF_CMD_STOPPED_FLAG                    (1 << 1)
#define RF_CMD_CANCELLED_FLAG                  (1 << 2)
#define RF_CMD_PREEMPT_FLAG                    (1 << 3)
#define RF_CMD_BG_CMD_FLAG                     (1 << 4)
#define RF_CMD_ALLOC_FLAG                      (1 << 7)
#define RF_CMD_CANCEL_MASK                     (RF_CMD_ABORTED_FLAG | RF_CMD_STOPPED_FLAG | RF_CMD_CANCELLED_FLAG)
#define RF_RUN_CMD_ON_RESTART_FLAG             (1 << 0)
#define RF_RAT_ERR_CB_EVENT_FLAG               (1 << 1)
#define RF_RAT_RPT_CPT_FLAG                    (1 << 2)
#define RF_HW_INT_RAT_CH_MASK                  (RFC_DBELL_RFHWIFG_RATCH7 | RFC_DBELL_RFHWIFG_RATCH6 | RFC_DBELL_RFHWIFG_RATCH5)
#define RF_MAX_RAT_CH_CNT                      3
#define RF_INTERNAL_HW_IFG_MASK_32_BIT         RFC_DBELL_RFHWIFG_MDMSOFT
/* default value for power up duration (in us) used before first power cycle */
#define RF_DEFAULT_POWER_UP_TIME               2500
/* default minimum power up duration (in us) */
#define RF_DEFAULT_MIN_POWER_UP_TIME           500
/* default power-up margin (in us) to account for wake-up sequence outside the RF power state machine */
#define RF_DEFAULT_POWER_UP_MARGIN             314
/* default power down duration in us */
#define RF_DEFAULT_POWER_DOWN_TIME             1000
#define RF_MAX_CHAIN_CMD_LEN                   32
#define RF_RAT_CH_5_INDEX                      0x0
#define RF_RAT_CH_6_INDEX                      0x1
#define RF_RAT_CH_7_INDEX                      0x2
/* RAT channel (0-4) are used by RF Core. Only 5,6,7 are available for application */
#define RF_RAT_CH_OFFSET                       5
#define RF_SEND_RAT_STOP_RATIO                 7
#define RF_RTC_CONV_TO_US_SHIFT                12
#define RF_SHIFT_32_BITS                       32
#define RF_SHIFT_8_BITS                        8
#define RF_SHIFT_16_BITS                       16
#define RF_RTC_TICK_INC                        (0x100000000LL/32768)
#define RF_SCALE_RTC_TO_4MHZ                   4000000
#define RF_NUM_RAT_TICKS_IN_1_US               4
/* (3/4)th of a full RAT cycle, in us */
#define RF_DISPATCH_MAX_TIME_US                (UINT32_MAX / RF_NUM_RAT_TICKS_IN_1_US * 3 / 4)
/* (1/4)th of a full RAT cycle, in us */
#define RF_DISPATCH_MAX_TIME_WRAPAROUND_US     (int32_t)(RF_DISPATCH_MAX_TIME_US - UINT32_MAX / RF_NUM_RAT_TICKS_IN_1_US)
#define RF_INTERNAL_CPE_IFG_MASK_32_BIT        0xFFFFFFFF
#define RF_XOSC_HF_SWITCH_CHECK_PERIOD         50
#define RF_DEFAULT_AVAILRATCH_VAL              0xFFFF
#define RF_ABORT_CANCEL_MULTIPLE_CMD           0x2
#define RF_ABORT_CANCEL_SINGLE_CMD             0x1
#define RF_CMDSTA_REG_VAL_MASK                 0xFF
#define RF_RAT_CPT_CONFIG_RATCH_MASK           0x0F00
#define RF_RAT_CPT_CONFIG_REP_MASK             0x1000
#define RF_PHY_SWITCHING_MODE                  1
#define RF_PHY_BOOTUP_MODE                     0
#define RF_SCH_CMD_PRI_PREEMPT_STOP            0x1
#define RF_SCH_CMD_PRI_PREEMPT_ABORT           0x2
#define RF_SCH_CMD_PRI_PREEMPT_ADD             0x3
#define RF_SCH_CMD_TIMING_INSERT               0x4
#define RF_REQ_ACCESS_MAX_DUR_US               1000000
/* Additional analog config time for setup command */
#define RF_ANALOG_CFG_TIME_US                  96
/* Update analog configuration in setup */
#define RF_SETUP_ANALOGCFG_UPDATE              0
/* Don't update analog configuration in setup */
#define RF_SETUP_ANALOGCFG_NOUPDATE            0x2D
#define RF_SCH_CMD_STARTTIME_NOW               0
#define RF_SCH_CMD_ENDTIME_IGNORE              0
#define RF_DEFAULT_PHY_SWITCHING_TIME          500
#define RF_RADIOFREECB_PREEMPT_FLAG            0x1
#define RF_RADIOFREECB_REQACCESS_FLAG          0x2
#define RF_RADIOFREECB_CMDREJECT_FLAG          0x4
#define RF_SCH_CMD_INSERT_QUEUE_TOP            0x1
#define RF_SCH_CMD_INSERT_QUEUE_LATER          0x2
#define RF_SCH_CMD_PREEMPT                     0x4
#define RF_DEFAULT_RAT_RTC_ERR_TOL_IN_US       5
/* approx for 1e6 / 500. XTAL drift is 500 ppm */
#define RF_DEFAULT_COMB_XTAL_DRIFT_BITS_SHIFT  11
/* window (in us) to decide if wakeup was from RF power up clock */
#define RF_WAKEUP_DETECTION_WINDOW_IN_US       100

/* RF command queue member*/
typedef struct RF_Cmd_s RF_Cmd;

/* RF command member*/
struct RF_Cmd_s {
    RF_Cmd* volatile     pNext;       /* Pointer to next #RF_Cmd in queue */
    RF_Callback volatile pCb;         /* Pointer to callback function */
    RF_Op*               pOp;         /* Pointer to (chain of) RF operations(s) */
    RF_Object*           pClient;     /* Pointer to client */
    RF_EventMask         bmEvent;     /* Enable mask for interrupts from the command */
    uint32_t             rfifg;       /* Return value for callback 0:31 - RF_CPE0_INT, 32:63 - RF_HW_INT */
    uint32_t             startTime;   /* command start time (in RAT ticks) */
    uint32_t             endTime;     /* command end time (in RAT ticks) */
    RF_CmdHandle         ch;          /* Command handle */
    RF_Priority          ePri;        /* Priority of RF command */
    uint8_t volatile     flags;       /* [0: Aborted, 1: Stopped, 2: cancelled] */
};

/* RF Direct Immediate command*/
typedef struct RF_CmdDirImm_s RF_CmdDirImm;

/* RF direct immediate command member*/
struct RF_CmdDirImm_s {
    RF_Handle            pClient[RF_MAX_RAT_CH_CNT];  /* Pointer to current client */
    RF_Callback          pRatCb[RF_MAX_RAT_CH_CNT];   /* Array of callback pointers for each RAT chan */
    uint32_t volatile    hwIntEvent;                  /* RF_HW events received */
    uint16_t             availRatCh;                  /* bit-map for available RAT channels */
    uint8_t              rptCptRatChNum;              /* Channel allocated for repeat mode */
    uint8_t              flags;                       /* [0:runOnRestart, 1:errEventCb, 2: rptCptFlag] */
};

static RF_CmdDirImm cmdDirImm;

/* RF power constraint */
static volatile uint8_t RF_powerConstraint;

/* FSM typedefs */
typedef void (*Fsm_StateFxn)(RF_Object*, Fsm_Event const);

/*-------------- Forward declarations ---------------*/

/* Common inactivity timeout clock callback */
static void clkInactivityFxn(RF_Object* pObj);

/* Common request access timeout clock callback */
static void clkReqAccessFxn(RF_Object* pObj);

/* FSM helper functions */
static void FsmDispatch(RF_Object* pObj, Fsm_Event const e);
static void FsmNextState(Fsm_StateFxn pTarget);

/* FSM state functions */
static void fsmPowerUpState(RF_Object *pObj, Fsm_Event e);
static void fsmSetupState(RF_Object *pObj, Fsm_Event e);
static void fsmActiveState(RF_Object *pObj, Fsm_Event e);
static void fsmXOSCState(RF_Object *pObj, Fsm_Event e);
static void fsmChangePhyState(RF_Object *pObj, Fsm_Event e);

/* RF Patching fxn */
static void applyRfCorePatch(bool mode);

/* RF direct and immediate command implementation */
static RF_Stat  postDirImmCmd(uint32_t pCmd, bool bStartRadio);

/*-------------- Constants ---------------*/

/*-------------- Global variables ---------------*/
/* Pointer to current radio client (indicates also whether radio powered) */
static RF_Object* pCurrClient;

/* Keeps track of whether RF Core is active */
static bool bRadioActive;

/* Keep track of module state */
static bool bInitialized = false;

/* Current Radio FSM state */
static Fsm_StateFxn pFsmState;

/* Count of radio clients */
static uint8_t nClients;

/* Common RF command queue shared by all clients */
static struct {
    RF_Cmd* volatile      pPend;     /* Queue of RF_Cmds pending execution */
    RF_Cmd* volatile      pCurrCmd;  /* RF_Cmd currently executing */
    RF_Cmd* volatile      pCurrCmdCb;/* Holds current command while callback is processed */
    RF_Cmd* volatile      pCurrCmdBg;/* Holds background command while foreground command is processed */
    RF_Cmd* volatile      pDone;     /* Queue of RF_Cmds that are done but not retired */
    RF_CmdHandle volatile nSeqPost;  /* Sequence # for previously posted command */
    RF_CmdHandle volatile nSeqDone;  /* Sequence # for last done command */
} cmdQ;

/* RF_Cmd container pool. Containers with extra information about RF commands in flight
   These are organized as a circular array rather than a "retired" or "free" queue to
   facilitate mapping to the sequence numbers used as command handles */
static RF_Cmd aCmdPool[N_CMD_POOL];

/* SWIs and HWIs structures */
static Swi_Struct swiFsm;
static void swiFxnFsm(xdc_UArg a, xdc_UArg b);
static Hwi_Struct hwiCpe0;
static Swi_Struct swiHw;
static void swiFxnHw(xdc_UArg a, xdc_UArg b);
static Hwi_Struct hwiHw;
static void hwiFxnHw(xdc_UArg a);

/* Functions for RF_CPE interrupt handling */
static void hwiFxnCpe0PowerFsm(xdc_UArg a);
static void hwiFxnCpe0Active(xdc_UArg a);
static void hwiFxnCpe0ChangePhy(xdc_UArg a);

/* Clock used for triggering power-up sequences */
static Clock_Struct clkPowerUp;
static void clkPowerUpFxn(xdc_UArg a);

/* Default error callback */
static void defaultErrCb(RF_Handle h, RF_CmdHandle ch, RF_EventMask e);

/* RAT sync start/stop radio operation command */
static union {
    rfc_CMD_SYNC_START_RAT_t start;
    rfc_CMD_SYNC_STOP_RAT_t  stop;
} opRatSync;

/* Default RF parameters structure */
static const RF_Params RF_defaultParams = {
    .nInactivityTimeout     = BIOS_WAIT_FOREVER,
    .nPowerUpDuration       = RF_DEFAULT_POWER_UP_TIME,
    .pPowerCb               = NULL,
    .pErrCb                 = defaultErrCb,
    .bPowerUpXOSC           = true,
    .nPowerUpDurationMargin = RF_DEFAULT_POWER_UP_MARGIN,
    .pClientEventCb         = NULL,
    .nClientEventMask       = 0,
};

/* variable to indicate power-up requested when powering down */
static bool volatile bRestart;

/* Internal abortCmd used to process RF_cancelCmd and RF_flushCmd */
static RF_Stat abortCmd(RF_Handle h, RF_CmdHandle ch, uint8_t type);

/* variables used for powerUpDuration, phySwitchingTime and RAT sync time calculation */
static struct {
    uint64_t nRtc1;             /* RTC timer value power-up and active time calculation */
    uint32_t nRtc2;             /* RTC timer value for switching time calculation */
    uint32_t activeTimeInUs;    /* total radio active time value since last power up (in us) */
    uint8_t  errTolValInUs;     /* max allowed error between RAT/RTC drift to enable resync at power-down (in us) */
} powerDurCalc;

/* Indicates if power down operation needs to send CMD_RAT_SYNC_STOP. Only used with RF_yield() */
static bool bRatSync;

/* Indicates the first instance the radio power up/down is done after rf_init() */
static bool bStart;

/* Hardware attribute structure populated in board.c file to set HWI and SWI priorities */
extern const RFCC26XX_HWAttrs RFCC26XX_hwAttrs;

/* RAT channel allocate fxn */
static int8_t ratChanAlloc(RF_CmdDirImm *pCmd);

/* RAT channel free fxn */
static void ratChanFree(int8_t ratChanNum, bool checkPowerDown);

/* Indicate when RF Core is in power-up FSM */
static bool bPowerUpActive;

/* Indicate when RF Core is in power-down FSM */
static bool bPowerDownActive;

/* Indicate if RF_yield requested to power-down the RF Core */
static bool bYieldPwrDnCalled;

/* Indicate request to cancel a power-down request from RF_yield */
static bool bYieldPwrDnCancel;

/* Indicate that switch change phy is active */
static bool bPowerChangePhy;

/* RF scheduler variables data structure */
static struct {
    RF_Handle       clientHnd[N_MAX_CLIENTS]; /* client handles for each registered client */
    uint32_t        phySwitchingTimeInUs[2];  /* phy switching time 0: client 1 -> 2, 1 : client 2 -> 1 */
    RF_AccessParams accReq[N_MAX_CLIENTS];    /* input parameters from any RF_requestAccess API calls */
    RF_Handle       clientHndRadioFreeCb;     /* client handle for the radio callback */
    /* structure to store items when scheduling results in preemption */
    struct {
        RF_Handle       client;               /* RF_Handle for the preempted client */
        RF_CmdHandle    cancelStartCmdHandle; /* RF_CmdHandle of the preemption start */
        bool            cancelType;           /* 1: flush gracefully, 0: flush abort */
    } preemptCmd;
    uint8_t         issueRadioFreeCbFlags;    /* indicate if driver needs to issue RF_EventRadioFree callback {0:pre-emption, 1:requestAccess running, 2: reject command} */
    uint8_t         cmdInsertFlags;           /* indicate if the command was inserted based on timing information */
} RF_Sch;

/* Obj wakeup notification from the Power driver */
static Power_NotifyObj RF_wakeupNotifyObj;

/* variable to indicate if the XOSC_HF_SWITCHING is done by the RF Driver */
static bool bXOSCSwitchConstraint;

/* variable to indicate with the FLASH is disable during the power up */
static bool bDisableFlashInIdleConstraint;

/*-------------- Command queue internal functions ---------------*/

/*
 *  Function: Q_peek
 *  Peek at first entry in simple queue
 *  Input:  ppHead - Pointer to the head of the command queue
 *  Return: RF command
 */
static RF_Cmd* Q_peek(RF_Cmd* volatile* ppHead)
{
    return (RF_Cmd*)*ppHead;
}

/*
 *  Function: Q_pop
 *  Pop first entry in simple queue
 *  Input:  ppHead - Pointer to the head of the command queue
 *  Return: RF command
 */
static RF_Cmd* Q_pop(RF_Cmd* volatile* ppHead)
{
    RF_Cmd* pCmd;
    uint32_t key = Hwi_disable();
    if ((pCmd=(RF_Cmd*)*ppHead))
    {
        *ppHead = (RF_Cmd*)pCmd->pNext;
    }
    Hwi_restore(key);
    return pCmd;
}

/*
 *  Function: Q_push
 *  Push entry onto simple queue
 *  Input: h      - RF handle
 *         ppHead - Pointer to the head of the command queue
 *         pCmd   - Pointer to command
 *  Return: true (success) or false (fail)
 */
static bool Q_push(RF_Cmd* volatile* ppHead, RF_Cmd* pCmd)
{
    RF_Cmd* pEnd = (RF_Cmd*)*ppHead;
    uint32_t key;

    pCmd->pNext = NULL;
    if (!pEnd)
    {
        /* First entry into queue */
        key = Hwi_disable();
        *ppHead = pCmd;
    }
    else
    {
        /* Find the tail */
        key = Hwi_disable();
        while (pEnd->pNext)
        {
            pEnd = (RF_Cmd*)pEnd->pNext;
        }

        /* Sanity check - to avoid pEnd->pNext pointing to self */
        if (pEnd == pCmd)
        {
            Hwi_restore(key);
            return false;
        }

        /* Append the command */
        pEnd->pNext = pCmd;
    }
    Hwi_restore(key);

    return true;
}

/*
 *  Function: Q_end
 *  Search last entry in simple queue  for particular client
 *  Input: h      - RF handle
 *         ppHead - Pointer to the head of the command queue
 *  Return: RF command
 */
static RF_Cmd* Q_end(RF_Handle h, RF_Cmd* volatile* ppHead)
{
    RF_Cmd* pEnd = NULL;
    RF_Cmd* pClientEnd = NULL;

    uint32_t key = Hwi_disable();
    if ((pEnd = (RF_Cmd*)*ppHead))
    {
        if (pEnd->pClient == h)
        {
            pClientEnd = pEnd;
        }
        while (pEnd->pNext)
        {
            pEnd = (RF_Cmd*)pEnd->pNext;
            if (pEnd->pClient == h)
            {
                pClientEnd = pEnd;
            }
        }
    }
    Hwi_restore(key);

    return pClientEnd;
}

/*
 *  Function: Q_first
 *  Search the first command in the queue from particular client
 *  Input: h      - RF handle
 *         ppHead - Pointer to the head of the command queue
 *  Return: RF command
 */
static RF_Cmd* Q_first(RF_Handle h, RF_Cmd* volatile* ppHead)
{
    RF_Cmd* pCmd;

    uint32_t key = Hwi_disable();
    pCmd = (RF_Cmd*)*ppHead;

    while (pCmd)
    {
        /* Check if command is from this client */
        if (pCmd->pClient == h)
        {
            /* Command is from this client, return the command */
            Hwi_restore(key);
            return pCmd;
        }
        else
        {
            /* Command is not from this client, check the next command */
            pCmd = pCmd->pNext;
        }
    }

    Hwi_restore(key);
    return NULL;
}

/*
 *  Function: allocCmd
 *  Allocate cmd buffer from cmd pool
 *  Input:  ch - RF command handle
 *  Return: RF command
 */
static RF_Cmd* allocCmd(RF_CmdHandle ch)
{
    uint8_t i;
    for (i = 0; i < N_CMD_POOL; i++)
    {
        /* Find the first available entry in the command pool */
        if (!(aCmdPool[i].flags & RF_CMD_ALLOC_FLAG))
        {
            return &aCmdPool[i];
        }
    }
    return NULL;
}

/*
 *  Function: getCmd
 *  Search cmd from cmd pool
 *  Input:  ch - RF command handle
 *  Return: RF command
 */
static RF_Cmd* getCmd(RF_CmdHandle ch)
{
    uint8_t i;
    for (i = 0; i < N_CMD_POOL; i++)
    {
        /* Find the allocated command pool entry corresponding to ch */
        if ((aCmdPool[i].flags & RF_CMD_ALLOC_FLAG) && (aCmdPool[i].ch == ch))
        {
            return &aCmdPool[i];
        }
    }
    return NULL;
}

/*  Function: RF_wakeupNotifyFxn
 *
 *  Wake up notification callback from the power driver. If the callback is from RF wakeup
 *  set constraint to let RF Driver control the XOSC switching else do nothing in the
 *  callback
 *
 *  Input:
 *
 *  Return: Power_NOTIFYDONE
 */
static uint8_t RF_wakeupNotifyFxn(uint8_t eventType, uint32_t *eventArg, uint32_t *clientArg)
{
    Clock_Handle powerUpClkHandle = Clock_handle(&clkPowerUp);

    /* check if the callback is for wakeup from standby and if power up clock is running */
    if ((eventType == PowerCC26XX_AWAKE_STANDBY) && (Clock_isActive(powerUpClkHandle)))
    {
        /* calculate time (in us) until next trigger (assume next trigger is max ~70 min away) */
        uint32_t timeInUsUntilNextTrig = Clock_tickPeriod * Clock_getTimeout(powerUpClkHandle);


        /* check if the next trig time is close enough to the actual power up */
        if (timeInUsUntilNextTrig < RF_WAKEUP_DETECTION_WINDOW_IN_US)
        {
            /* stop power up clock */
            Clock_stop(powerUpClkHandle);
            /* setup RF Driver to do the XOSC_HF switching */
            Power_setConstraint(PowerCC26XX_DISALLOW_XOSC_HF_SWITCHING);
            /* set variable to indicate RF Driver will do the XOSC_HF switching */
            bXOSCSwitchConstraint = true;
            /* start the RF Core power up */
            Swi_or(Swi_handle(&swiFsm), Fsm_EventWakeup);
        }
    }

    return Power_NOTIFYDONE;
}

/*-------------- Scheduler internal functions --------------------------------*/

/*  Function: RF_issueRadioFreeCb
 *  Issue RF_EventRadioFree callback to the client. The callback is issued -
 *  1. After pre-emption is complete
 *  2. Dedicated request access period expires or released
 *  3. command reject because of other high priority command running
 *  Input:  src - flag indicating the source of callback request
 *  Return: none
 */
static void RF_issueRadioFreeCb(uint8_t src)
{
    uint32_t key = Hwi_disable();
    RF_Sch.issueRadioFreeCbFlags &= ~src;

    bool isReqAccessActive = false;
    if (RF_Sch.clientHnd[0])
    {
        isReqAccessActive |= Clock_isActive(Clock_handle(&RF_Sch.clientHnd[0]->state.clkReqAccess));
    }
    if (RF_Sch.clientHnd[1])
    {
        isReqAccessActive |= Clock_isActive(Clock_handle(&RF_Sch.clientHnd[1]->state.clkReqAccess));
    }

    if ((RF_Sch.issueRadioFreeCbFlags == 0) && !isReqAccessActive)
    {
        if (RF_Sch.clientHndRadioFreeCb)
        {
            if ((RF_Sch.clientHndRadioFreeCb->clientConfig.pClientEventCb) &&
                (RF_Sch.clientHndRadioFreeCb->clientConfig.nClientEventMask & RF_ClientEventRadioFree))
            {
                RF_ClientCallback pClientEventCb = (RF_ClientCallback)RF_Sch.clientHndRadioFreeCb->clientConfig.pClientEventCb;
                Hwi_restore(key);
                pClientEventCb(RF_Sch.clientHndRadioFreeCb, RF_ClientEventRadioFree, NULL);
                RF_Sch.clientHndRadioFreeCb = NULL;
            }
            else if (RF_Sch.clientHndRadioFreeCb->clientConfig.pPowerCb)
            {
                RF_Callback pPowerCb = (RF_Callback)RF_Sch.clientHndRadioFreeCb->clientConfig.pPowerCb;
                Hwi_restore(key);
                pPowerCb(RF_Sch.clientHndRadioFreeCb, 0, RF_EventRadioFree);
                RF_Sch.clientHndRadioFreeCb = NULL;
            }
        }
    }
    else
    {
        Hwi_restore(key);
    }
}
/*
 *  Function: RF_schChkCmdInsert
 *  Check if new request can inserted between the previous and next command in the
 *  current queue.
 *  Input - newCmd : RF_Cmd pointer for the new command request
 *          prevCmd : RF_Cmd pointer for the previous cmd in the queue
 *          nextCmd : RF_Cmd pointer for the next cmd in the queue
 *  Return - bool  : return true if command can be inserted in the queue else
 *                   return false.
 */
static bool RF_schChkCmdInsert(RF_Cmd* newCmd, RF_Cmd* prevCmd, RF_Cmd* nextCmd)
{
    bool insertNewCmdAfterPrev, insertNewCmdBeforeNext;
    int32_t deltaInUs, clientSwitchingTime;

    /* Initalize global variables */
    insertNewCmdAfterPrev = false;
    insertNewCmdBeforeNext = false;
    if (!nextCmd)
    {
        insertNewCmdBeforeNext = true;
    }
    clientSwitchingTime = 0;

    /*
        Check if newCmd can be inserted between prevCmd and nextCmd.
        if prevCmd is not NULL check if startTime of newCmd is more endTime of the
        prevCmd.
        if nextCmd is not NULL check if the endTime of the newCmd is less than the
        startTime of the nextCmd.
    */
    if ((prevCmd) && (prevCmd->endTime != RF_SCH_CMD_ENDTIME_IGNORE) && (newCmd->startTime != RF_SCH_CMD_STARTTIME_NOW))
    {
        /* if prevCmd and newCmd are from different client then switching time needs to be included in deltaInUs calc */
        if (prevCmd->pClient != newCmd->pClient)
        {
            if (prevCmd->pClient == RF_Sch.clientHnd[0])
            {
                clientSwitchingTime = RF_Sch.phySwitchingTimeInUs[0];
            }
            else
            {
                clientSwitchingTime = RF_Sch.phySwitchingTimeInUs[1];
            }
        }

        deltaInUs = ((int32_t)(newCmd->startTime >> 2) - ((int32_t)(prevCmd->endTime >> 2) + clientSwitchingTime));
        /*
           check if startTime or endTime has overflowed
           Limitation: Assumes that difference between the startTime and endTime is less than 8 min
        */
        if (deltaInUs >= (int32_t)RF_DISPATCH_MAX_TIME_US)
        {
            /* endTime has wrapped around */
            return 0;
        }
        else if ((deltaInUs < ((int32_t)RF_DISPATCH_MAX_TIME_WRAPAROUND_US)) || (deltaInUs > 0))
        {
            /* startTime has wrapped around or no wrap around and we can insert the command */
            insertNewCmdAfterPrev = true;
        }
        else
        {
            return 0;
        }
    }

    if ((nextCmd) && (nextCmd->startTime != RF_SCH_CMD_STARTTIME_NOW) && (newCmd->endTime != RF_SCH_CMD_ENDTIME_IGNORE))
    {
        clientSwitchingTime = 0;
        /* if nextCmd and newCmd are from different client then switching time needs to be included in deltaInUs calc */
        if (newCmd->pClient != nextCmd->pClient)
        {
            if (nextCmd->pClient == RF_Sch.clientHnd[0])
            {
                clientSwitchingTime = RF_Sch.phySwitchingTimeInUs[0];
            }
            else
            {
                clientSwitchingTime = RF_Sch.phySwitchingTimeInUs[1];
            }
        }

        deltaInUs = (((int32_t)(nextCmd->startTime >> 2) - clientSwitchingTime) - (int32_t)(newCmd->endTime >> 2));
        /*
            check if startTime or endTime has overflowed.
            Limitation: Assumes that difference between the startTime and endTime is less than 8 min
        */
        if (deltaInUs >= (int32_t)RF_DISPATCH_MAX_TIME_US)
        {
            /* endTime has wrapped around */
            return 0;
        }
        else if ((deltaInUs < ((int32_t)RF_DISPATCH_MAX_TIME_WRAPAROUND_US)) || (deltaInUs > 0))
        {
            /* startTime has wrapped around or no wrap around and we can insert the command */
            insertNewCmdBeforeNext = true;
        }
        else
        {
            return 0;
        }
    }

    return (insertNewCmdBeforeNext & insertNewCmdAfterPrev);
}

/*
 *  Function: RF_schPriorityCompare
 *  Check newReqPriority can preempt the currPriority
 *  Input - currPriority : RF_Prioirty of the existing command
 *          newReqPriority : RF_Priority of the new request
 *  Return - int8_t : indicate if new request priority can preempt the existing
 *                    command and the preemption type.
 */
static int8_t RF_schPriorityCompare(RF_Priority currPriority, RF_Priority newReqPriority)
{
    if (newReqPriority <= currPriority)
    {
        return 0;
    }
    else if (newReqPriority == RF_PriorityHigh)
    {
        return RF_SCH_CMD_PRI_PREEMPT_STOP;
    }
    else
    {
        return RF_SCH_CMD_PRI_PREEMPT_ABORT;
    }
}

/*
 *  Function: RF_schCmdPriority
 *  Scan through the command queue to check new command with RF_Priority = priority
 *  can be added by preempted existing commands in the queue.
 *  Input - h2       : RF_Handle of client not requesting the new command
 *          priority : RF_Priority of the new request
 *  Return value - 1 : new request can be granted, required cmds from h2 are cancelled
 *                 2 : no command from client h2, new command can be added
 *                 0 : new command priority cannot preempt any command in current queue
 */
static uint8_t RF_schCmdPriority(RF_Handle h2, RF_Priority priority)
{
    /* local variables declaration */
    RF_Cmd* pCmdSearchStart;
    int8_t cmdQReq, cmdQReq_curr;
    RF_Cmd* pPreemptCmd;
    bool noCmdFromClient2;

    /* initialize local variables */
    /* priority weights while comparing each command priority */
    cmdQReq = 0;
    cmdQReq_curr = 0;
    /* pCmdSearchStart holds temp pointer to the cmd in the queue whose priority is
       compared with the priority of the new request */
    pCmdSearchStart = NULL;
    /* pPreemptCmd holds pointer to the first command in the queue from client h2
       which needs to be preempted */
    pPreemptCmd = NULL;
    /* no command from client h2 in the current queue */
    noCmdFromClient2 = true;

    /*
         currCmd  pendCmd
      1.   NULL   NULL        Add to end of queue
      2.   NULL   all h1      Add to end of queue
      3.   NULL   h1 and h2   Check cmd parameters
      4.   h1     NULL        Add to end of queue
      5.   h1     all h1      Add to end of queue
      6.   h1     h1 and h2   Check cmd parameters
      7.   h2     NULL        Check cmd parameters
      8.   h2     all h1      Add to end of queue
      9.   h2     h1 and h2   Check cmd parameters

       search through all the radio commands in the aCmdPool (current command and pending commands)
       to decide if the new command requires pre-empting any commands or simply add to end of the
       queue.
       Commands are preempted only if priority of the new request is greater than ALL the commands
       in the current command from client h2.
    */

    /* check the current running command first */
    if ((cmdQ.pCurrCmd) && (cmdQ.pCurrCmd->pClient == h2))
    {
        cmdQReq = RF_schPriorityCompare(cmdQ.pCurrCmd->ePri, priority);
        noCmdFromClient2 = false;
        if (cmdQReq)
        {
            pPreemptCmd = cmdQ.pCurrCmd;
        }
    }

    if (cmdQ.pPend)
    {
        pCmdSearchStart = cmdQ.pPend;

        /* check the pending queue */
        while(pCmdSearchStart)
        {
            if (pCmdSearchStart->pClient == h2)
            {
                cmdQReq_curr = RF_schPriorityCompare(pCmdSearchStart->ePri, priority);
                noCmdFromClient2 = false;
                if ((cmdQReq_curr != 0) && (cmdQReq == 0))
                {
                    pPreemptCmd = pCmdSearchStart;
                }

                if (((cmdQReq_curr != 0) && (cmdQReq == 0)) ||
                    (cmdQReq_curr == 0))
                {
                    cmdQReq = cmdQReq_curr;
                }
            }
            pCmdSearchStart = pCmdSearchStart->pNext;
        }
    }

    if (noCmdFromClient2)
    {
        /* there is no command from h2 in the queue, so no need for preemption */
        return 2;
    }

    if ((cmdQReq) && (pPreemptCmd))
    {
        /* set command preempt flag, used to issue RF_EventCmdPreempted in the cmd cancel cb */
        RF_Cmd* pTmpCmd = pPreemptCmd;
        if ((cmdQ.pCurrCmd->pClient == h2) && (pTmpCmd == cmdQ.pCurrCmd))
        {
            cmdQ.pCurrCmd->flags |= RF_CMD_PREEMPT_FLAG;
            pTmpCmd = cmdQ.pPend;
        }
        while(pTmpCmd)
        {
            if (pTmpCmd->pClient == h2)
            {
                pTmpCmd->flags |= RF_CMD_PREEMPT_FLAG;
            }
            pTmpCmd = pTmpCmd->pNext;
        }

        RF_Sch.clientHndRadioFreeCb = h2;
        /* set flag to issue the RF_EventRadioFree once the new command(s) from h2 is done */
        RF_Sch.issueRadioFreeCbFlags |= RF_RADIOFREECB_PREEMPT_FLAG;

        /* save the variables needed to call RF_flushCmd() because of preemption */
        RF_Sch.preemptCmd.client = h2;
        RF_Sch.preemptCmd.cancelStartCmdHandle = pPreemptCmd->ch;
        RF_Sch.preemptCmd.cancelType = cmdQReq & 0x1;

        return 1;
    }
    else
    {
        /* preemption not possible */
        return 0;
    }
}

/*
 *  Function: RF_schCmdRunInsertPreempt
 *  Check if new command can be scheduled based on to the current command queue
 *  and the request access
 *  Input-  h1 : RF_Handle of requesting command
 *          h2 : RF_Handle of the client not requesting the new command
 *         pOp : RF_Op pointer for the requesting command
 *  pSchParams : Pointer to user input RF_ScheduleCmdParams structure of the requesting command
 *
 *  Output- pSchStatus: pointer to schedule error
 *
 *  Return- NULL : If new command request cannot be met
 *          RF_Cmd* : valid aCmdPool pointer to the slot allocated for the new cmd request
 */
static RF_Cmd* RF_schCmdRunInsertPreempt(RF_Handle h1, RF_Handle h2, RF_Op* pOp, RF_ScheduleCmdParams *pSchParams,
                                         RF_ScheduleCmdStatus *pSchStatus)
{
    RF_Cmd* pCmd;
    RF_Cmd* pTmp;
    RF_CmdHandle ch;
    uint8_t bPrioritySch;

    /* initalize local variables */
    pCmd = NULL;
    bPrioritySch = 0;

    /* initialize global variables */
    RF_Sch.cmdInsertFlags = 0;

    /* initialize schedule command status */
    *pSchStatus = RF_ScheduleCmdSuccess;

    /*
       check if command queue has free entries and allocate RF_Op* container
       if command queue is full reject the command
    */
    if (((cmdQ.nSeqPost + N_CMD_MODMASK + 1 - cmdQ.nSeqDone) & N_CMD_MODMASK) < N_CMD_POOL)
    {
        ch = (cmdQ.nSeqPost + 1) & N_CMD_MODMASK;
        pCmd = allocCmd(ch);
        pCmd->ch = ch;

        /* cancel the radio free callback if new command is from the same client */
        if ((RF_Sch.clientHndRadioFreeCb == h1) &&
            (RF_Sch.issueRadioFreeCbFlags & RF_RADIOFREECB_PREEMPT_FLAG))
        {
            RF_Sch.issueRadioFreeCbFlags &= ~RF_RADIOFREECB_PREEMPT_FLAG;
        }

        /* update the input timing information to the command */
        if (pSchParams->endTime)
        {
            pCmd->endTime = pSchParams->endTime;
        }
        else
        {
            pCmd->endTime = RF_SCH_CMD_ENDTIME_IGNORE;
        }

        if (pOp->startTrigger.triggerType == TRIG_ABSTIME)
        {
            pCmd->startTime = pOp->startTime;
        }
        else
        {
            pCmd->startTime = RF_SCH_CMD_STARTTIME_NOW;
        }

        /* No need to run scheduler if only one client is setup */
        if (!h2)
        {
            return pCmd;
        }
    }
    else
    {
        *pSchStatus = RF_ScheduleCmdAllocError;
        return NULL;
    }

    /*
       If there is RF_requestAccess (currently always RF_PriorityHighest)
       from a different client running, reject any new commands from client h1
    */
    if (h2 && (Clock_isActive(Clock_handle(&h2->state.clkReqAccess))))
    {
        *pSchStatus = RF_ScheduleCmdSchError;
        return NULL;
    }

    /*
       Scheduler resolution order -
       1a. Check if new command can be inserted based on the timing information
           at the top of the pending queue .
       1b. Check if new command can be inserted based on the timing information
           in the middle/end of the pending queue.
       2.  Check if new command can preempt existing commands based on priority.
       3.  Check if new command can be appended to the end of pending queue.
    */

    /* Step 1a */
    pTmp = Q_end(h1, &cmdQ.pPend);

    if (!pTmp && cmdQ.pPend)
    {
        RF_Cmd tmpCmd;
        tmpCmd.endTime = RF_getCurrentTime();

        if ((cmdQ.pCurrCmd && RF_schChkCmdInsert(pCmd, cmdQ.pCurrCmd, cmdQ.pPend)) ||
            (!cmdQ.pCurrCmd && RF_schChkCmdInsert(pCmd, &tmpCmd, cmdQ.pPend)))
        {
            /* insert command at the beginning of the queue */
            pCmd->pNext = cmdQ.pPend;
            cmdQ.pPend = pCmd;
            RF_Sch.cmdInsertFlags = RF_SCH_CMD_INSERT_QUEUE_TOP;
        }
    }

    /* Step 1b */
    if (!RF_Sch.cmdInsertFlags)
    {
        if (!pTmp)
        {
            pTmp = cmdQ.pPend;
        }
        while (pTmp)
        {
            if (RF_schChkCmdInsert(pCmd, pTmp, pTmp->pNext))
            {
                /* insert command between pTmp and pTmp->pNext */
                pCmd->pNext = pTmp->pNext;
                pTmp->pNext = pCmd;
                RF_Sch.cmdInsertFlags = RF_SCH_CMD_INSERT_QUEUE_LATER;
                break;
            }
            else
            {
                pTmp = pTmp->pNext;
            }
        }
    }

    /* Step 2 */
    if (!RF_Sch.cmdInsertFlags)
    {
        bPrioritySch = RF_schCmdPriority(h2, pSchParams->priority);
        if (bPrioritySch == 1)
        {
            RF_Sch.cmdInsertFlags = RF_SCH_CMD_PREEMPT;
        }
    }

    /* Step 3 */
    if (!RF_Sch.cmdInsertFlags && (bPrioritySch != 1))
    {
        /* traverse to the end of the command queue */
        pTmp = cmdQ.pPend;
        if (pTmp)
        {
            while(pTmp->pNext)
            {
                pTmp = pTmp->pNext;
            }
        }

        /* If new command has TRIG_ABSTIME triggerType and pastTrig = 0 ->
           check if the timing can be meet to append to command to the end of the queue,
           reject command is cannot be appended */
        if ((pOp->startTrigger.triggerType == TRIG_ABSTIME) && !pOp->startTrigger.pastTrig)
        {
            if ((pTmp && !RF_schChkCmdInsert(pCmd, pTmp, NULL)) ||
                (!pTmp && cmdQ.pCurrCmd && !RF_schChkCmdInsert(pCmd, cmdQ.pCurrCmd, NULL)))
            {
                *pSchStatus = RF_ScheduleCmdSchError;
                return NULL;
            }
        }
    }

    return pCmd;
}

/*
 *  Function: RF_schReqAccessCheck
 *  Check if new access request can be granted
 *  Input   h1 : RF_Handle of requesting command
 *          h2 : RF_Handle of the client not requesting the new command
 *  Return  0 : Request cannot be granted
 *          1 : New request can be granted
 */
static uint8_t RF_schReqAccessCheck(RF_Handle h1, RF_Handle h2)
{
    /* reject access request if any request is ongoing */
    if ((Clock_isActive(Clock_handle(&h1->state.clkReqAccess))) ||
        (h2 && (Clock_isActive(Clock_handle(&h2->state.clkReqAccess)))))
    {
        return 0;
    }

    if (h2)
    {
        return RF_schCmdPriority(h2, RF_PriorityHighest);
    }
    else
    {
        return 2;
    }
}

/*
 *  Function: RF_executePowerDown
 *  Execute RF power dwon sequence
 *  Input:
 *  Return:
 */
static void RF_executePowerDown()
{
    /* Powerdown RF core
       The proper thing to do here is to avoid switching off the digital part of
       radio without switching off the analog parts first */

    /* set VCOLDO reference */
    RFCAdi3VcoLdoVoltageMode(false);

    /* Remap HWI to startup function */
    Hwi_setFunc(Hwi_handle(&hwiCpe0), hwiFxnCpe0PowerFsm, (UArg)NULL);

     /* find the radio core active time since the last power up */
    powerDurCalc.activeTimeInUs = (AONRTCCurrent64BitValueGet() - powerDurCalc.nRtc1) >> RF_RTC_CONV_TO_US_SHIFT;

    /* Decide whether to send the CMD_SYNC_STOP_RAT command. If this is first power down (bStart is false), active time
       is longer than the time that can cause max allowed error between RAT and RTC clocks or if RF_yield was called
       (bRatSync is true) */
    if (!bStart || powerDurCalc.activeTimeInUs > (powerDurCalc.errTolValInUs << RF_DEFAULT_COMB_XTAL_DRIFT_BITS_SHIFT) ||
        bRatSync)
    {
        bStart = 1;

        bRatSync = false;

        /* Setup RAT_SYNC command to follow FsPowerdown command */
        opRatSync.stop.commandNo                 = CMD_SYNC_STOP_RAT;
        opRatSync.stop.condition.rule            = COND_NEVER;
        opRatSync.stop.startTrigger.triggerType  = TRIG_NOW;
        opRatSync.stop.pNextOp                   = NULL;

        uint32_t key = Hwi_disable();

        /* Disable CPE last command done interrupt */
        HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFCPEIEN) &= ~RFC_DBELL_RFCPEIEN_LAST_COMMAND_DONE_M;

        /* Clear CPE last command done interrupt flag */
        HWREG(RFC_DBELL_BASE+RFC_DBELL_O_RFCPEIFG) = ~RFC_DBELL_RFCPEIFG_LAST_COMMAND_DONE_M;

        /* Send RAT Stop command */
        HWREG(RFC_DBELL_BASE + RFC_DBELL_O_CMDR) = (uint32_t)&opRatSync.stop;

        /* Wait till RAT Stop command to complete */
        while (!(HWREG(RFC_DBELL_BASE+RFC_DBELL_O_RFCPEIFG) & RFC_DBELL_RFCPEIFG_LAST_COMMAND_DONE_M));

        /* Clear CPE last command done interrupt flag and enable last command done interrupt */
        HWREG(RFC_DBELL_BASE+RFC_DBELL_O_RFCPEIFG) = ~RFC_DBELL_RFCPEIFG_LAST_COMMAND_DONE_M;
        HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFCPEIEN) |= RFC_DBELL_RFCPEIEN_LAST_COMMAND_DONE_M;

        Hwi_restore(key);
    }

    /* reset nRtc1 */
    powerDurCalc.nRtc1 = 0;

    /* reset the active time */
    powerDurCalc.activeTimeInUs = 0;

    /* from powerdown state */
    /* Turn off Synth */
    RFCSynthPowerDown();

    /* Closing all handles */
    if (!bInitialized)
    {
        Semaphore_post(Semaphore_handle(&pCurrClient->state.semSync));
    }

    /* Turn off CPE */
    RFCClockDisable();
}

/*-------------- Power constraints internal functions ------------------------*/

/*
 * Function: RF_setPowerConstraint
 * Set RF power constraints.
 * Input:  src - RF_PowerConstraintSrc (Source: Queue or RAT)
 * Return: status
 */
RF_Stat RF_setPowerConstraint(RF_PowerConstraintSrc src)
{
    uint32_t key = Hwi_disable();

    /* Set constraint based on source */
    RF_powerConstraint |= src;

    Hwi_restore(key);

    return RF_StatSuccess;
}

/*
 * Function: RF_releasePowerConstraint
 * Release RF power constraints.
 * Input:  src - RF_PowerConstraintSrc (Source: Queue or RAT)
 * Return: status
 */
RF_Stat RF_releasePowerConstraint(RF_PowerConstraintSrc src)
{
    /* Check if in power down already */
    uint32_t key = Hwi_disable();
    if (RF_powerConstraint == 0)
    {
        /* No need to do anything if all constraints are clear */
        Hwi_restore(key);
        return RF_StatRadioInactiveError;
    }
    else
    {
        /* Release this constraint */
        RF_powerConstraint &= ~src;

        /* Check if all constraints are clear */
        if (RF_powerConstraint == 0)
        {
            Hwi_restore(key);
            /* Start power down if all constraints are clear */
            Swi_or(Swi_handle(&swiFsm), Fsm_EventPowerDown);
            return RF_StatSuccess;
        }
        else
        {
            Hwi_restore(key);
            return RF_StatSuccess;
        }
    }
}

/*
 *  Function: dispatchOpWhen
 *  Check whether to dispatch operation immediately or power down radio and wait.
 *  If the former it returns 0, if the latter it returns the #us until when RF
 *  need to execute the command
 *
 *  Calling context: Hwi, Swi
 *
 *  Input:   pOp - Pointer to radio operation command structure
 *
 *  Return:  true  if command is far away in future.
 *           false if command is too close and should be scheduled now.
 */
static bool dispatchOpWhen(RF_Op* pOp)
{
    uint32_t nPowerUpDuration;

    /* Only recognizes TRIG_ABSTIME triggers, everything else gets dispatched immediately */
    if (pOp->startTrigger.triggerType == TRIG_ABSTIME)
    {
        /* Difference of startTime from current time in us */
        uint32_t deltaTimeUs = (pOp->startTime - RF_getCurrentTime()) / RF_NUM_RAT_TICKS_IN_1_US;

        /* If pCurrClient is NULL (very initial state), use default powerup duration */
        if (pCurrClient)
        {
            nPowerUpDuration = pCurrClient->clientConfig.nPowerUpDuration;
        }
        else
        {
            nPowerUpDuration = RF_DEFAULT_POWER_UP_TIME;
        }

        /* Check if delta time is greater than (powerup duration + power down duration) for a
           power cycle, and is less than 3/4 of a RAT cycle (~17 minutes) */
        if ((deltaTimeUs > (int32_t)(nPowerUpDuration + RF_DEFAULT_POWER_DOWN_TIME)) &&
            (deltaTimeUs <= RF_DISPATCH_MAX_TIME_US))
        {
            /* Delta time is within range */
            Clock_setTimeout(Clock_handle(&clkPowerUp),
                             MAX((deltaTimeUs - nPowerUpDuration)/ Clock_tickPeriod, 1));
            return 1;
        }
    }
    return 0;
}

/*
 *  Function: RF_isBgCmdActive
 *  Check whether background command is currently active
 *  and RF mode is in IEEE 15.4 mode
 *
 *  Calling context: Hwi
 *
 *  Input:   pCmd - Pointer to current command
 *           pBgCmd - Pointer to background command
 *
 *  Return:  true  if no background command is running.
 *           false if background command is running.
 */
static bool RF_isBgCmdActive(RF_Cmd *pCmd, RF_Cmd *pBgCmd)
{
    /* Avoid device power down if background command is running (apply to IEEE 15.4 mode) */
    if((pCmd->pClient->clientConfig.pRfMode->rfMode == RF_MODE_IEEE_15_4) &&
       (pBgCmd != NULL))
    {
        return 0;
    }
    else
    {
        return 1;
    }
}


/*
 *  Function: updateImplicitState
 *
 *  Update implicit state based on finished commands
 *
 *  Calling context: Hwi, Swi
 *
 *  Input:   pCmd - Pointer to radio operation command
 *
 *  Return:
 */
static void updateImplicitState(RF_Cmd* pCmd)
{
    uint8_t nCmdChainMax = RF_MAX_CHAIN_CMD_LEN;

    /* Looks at operation or chained operations in a command and:
      - Makes a copy of any FS operation for a future power up sequence */
    RF_Op* pOp = pCmd->pOp;
    while (pOp && nCmdChainMax)
    {
        if ((pOp->commandNo == CMD_FS) || (pOp->commandNo == CMD_FS_OFF))
        {
            /* Create a copy of CMD_FS command (or CMD_FS_OFF) for later power up */
            memcpy(&pCmd->pClient->state.mode_state.cmdFs, pOp, sizeof(pCmd->pClient->state.mode_state.cmdFs));
            break;
        }
        pOp = pOp->pNextOp;
        --nCmdChainMax;
    }
}

/*
 *  Function: RF_dispatchCmd
 *
 *  submit the pending command to the RF Core
 *
 */
static void RF_dispatchCmd(RF_Cmd* pCmd)
{
    if (pCmd->pClient != pCurrClient)
    {
        /* We need to change radio client, signal to FSM. */
        Swi_or(Swi_handle(&swiFsm), Fsm_EventInitChangePhy);
    }
    else if (dispatchOpWhen(pCmd->pOp) && RF_isBgCmdActive(pCmd, cmdQ.pCurrCmdBg))
    {
        /* Command sufficiently far into future that it shouldn't be dispatched yet
           Release RF power constraint and potentially power down radio */
        if (RF_releasePowerConstraint(RF_PowerConstraintCmdQ) == RF_StatSuccess)
        {
            /* Stop inactivity timer */
            Clock_stop(Clock_handle(&pCurrClient->state.clkInactivity));

            /* Start the clock to trigger radio wakeup */
            Clock_start(Clock_handle(&clkPowerUp));
        }
    }
    else
    {
        HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFCPEIEN) |= (pCmd->bmEvent & RF_INTERNAL_CPE_IFG_MASK_32_BIT);
        HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFHWIEN) |= (pCmd->bmEvent >> RF_SHIFT_32_BITS);

        /* Dispatch immediately, set command to current and remove from pending queue */
        HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFACKIFG) = 0;
        HWREG(RFC_DBELL_BASE+RFC_DBELL_O_CMDR) = (uint32_t)pCmd->pOp;
        while(!HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFACKIFG));

        /* command sent to CPE -> move it from pending queue to the current command */
        Q_pop(&cmdQ.pPend);
        cmdQ.pCurrCmd = pCmd;

        /* Check pend queue for any foreground command (IEEE 15.4 mode) */
        if(cmdQ.pCurrCmd->pClient->clientConfig.pRfMode->rfMode == RF_MODE_IEEE_15_4)
        {
            /* Post SWI to handle for any foreground command */
            Swi_or(Swi_handle(&swiFsm), Fsm_EventIeeeInt);
        }
    }
}

/*
 *  Function: RF_checkCmdFsError
 *
 *  Check if there was an error with the synth while running CMD_FS
 *  error callback is not issued in this function.
 *
 *  Return: True if there was an error
 *          False if there was no error
 */
static bool RF_checkCmdFsError(void)
{
    RF_Handle pObj = pCurrClient;

    if ((pObj->clientConfig.pErrCb) && ((RF_Callback)pObj->clientConfig.pErrCb != defaultErrCb))
    {
        RF_Op *tmp1 = (RF_Op*)&pObj->clientConfig.pOpSetup->prop;
        while (tmp1->pNextOp && tmp1->pNextOp != (RF_Op*)&pObj->state.mode_state.cmdFs)
        {
            tmp1 = tmp1->pNextOp;
        }
        if ((tmp1->condition.rule == COND_ALWAYS) &&
            (pObj->state.mode_state.cmdFs.status == ERROR_SYNTH_PROG))
        {
            /* CMD_FS completed with error so return true */
            return true;
        }
    }

    /* there is no synth error so return false */
    return false;
}

/*
 *  Function: hwiFxnHw
 *
 *  RF HW ISR when radio is active
 *
 */
static void hwiFxnHw(xdc_UArg a)
{
    RF_CmdDirImm* pDirImmCmd = &cmdDirImm;

    uint32_t rfchwifg = (HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFHWIFG) & (RF_INTERNAL_HW_IFG_MASK_32_BIT | RF_HW_INT_RAT_CH_MASK));
    HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFHWIFG) = ~rfchwifg;

    /* Post SWI to handle registered callbacks */
    if ((rfchwifg & RF_INTERNAL_HW_IFG_MASK_32_BIT) &&
        (HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFHWIEN) & RF_INTERNAL_HW_IFG_MASK_32_BIT))
    {
        Swi_or(Swi_handle(&swiFsm), Fsm_EventHwInt);
    }

    if (rfchwifg & RF_HW_INT_RAT_CH_MASK)
    {
        pDirImmCmd->hwIntEvent |= (rfchwifg & RF_HW_INT_RAT_CH_MASK);
        Swi_or(Swi_handle(&swiHw), 0);
    }
}

/*
 *  Function: swiFxnHw
 *
 *  RF HW SWI used for user callback to any RAT event (RF_HW HWI)
 *
 */
static void swiFxnHw(xdc_UArg a, xdc_UArg b)
{
    RF_CmdDirImm* pDirImmCmd = &cmdDirImm;

    uint32_t key = Hwi_disable();
    if (pDirImmCmd->hwIntEvent & RF_HW_INT_RAT_CH_MASK)
    {
        uint8_t ratCbIdx = pDirImmCmd->hwIntEvent >> RFC_DBELL_RFHWIFG_RATCH5_BITN;

        /* Process lower channel first and allow multiple interrupt flags to be processed sequentially */
        if (ratCbIdx & (1 << RF_RAT_CH_5_INDEX))
        {
            ratCbIdx = RF_RAT_CH_5_INDEX;
        }
        else if (ratCbIdx & (1 << RF_RAT_CH_6_INDEX))
        {
            ratCbIdx = RF_RAT_CH_6_INDEX;
        }
        else if (ratCbIdx & (1 << RF_RAT_CH_7_INDEX))
        {
            ratCbIdx = RF_RAT_CH_7_INDEX;
        }

        /* free RAT channel but do not release RAT constraint yet */
        ratChanFree(ratCbIdx + RF_RAT_CH_OFFSET, false);
        pDirImmCmd->hwIntEvent &= ~(1 << (ratCbIdx + RFC_DBELL_RFHWIFG_RATCH5_BITN));
        Hwi_restore(key);

        /* issue RAT callback if setup */
        if (pDirImmCmd->pRatCb[ratCbIdx])
        {
            pDirImmCmd->pRatCb[ratCbIdx](pDirImmCmd->pClient[ratCbIdx],
                                         ratCbIdx + RF_RAT_CH_OFFSET,
                                         RF_EventRatCh);
        }

        /* check for any pending RAT channels running and release RAT power constraint */
        if (!HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFHWIEN))
        {
            RF_releasePowerConstraint(RF_PowerConstraintRat);
        }
    }
    else
    {
        Hwi_restore(key);
    }

    /* Process if multiple interrupt flag bits are set */
    if (pDirImmCmd->hwIntEvent)
    {
        Swi_or(Swi_handle(&swiHw), 0);
    }
}

/*
 *  Function: hwiFxnCpe0Active
 *
 *  RF CPE0 ISR when radio is active. Assume that all IRQs relevant to command
 *  dispatcher are mapped here. Furthermore, assume that there is no need for
 *  critical sections here (i.e. that this ISR has higher priority than
 *  any HWI calling a RF API function or that HWIs can't call the RF API)
 *
 */
static void hwiFxnCpe0Active(xdc_UArg a)
{
    RF_Cmd* pCmd;

    /* Get and clear doorbell IRQs */
    uint32_t rfcpeifg = RFCCpeIntGetAndClear();

    /* Retire command that has run */
    if (rfcpeifg & (RFC_DBELL_RFCPEIFG_LAST_COMMAND_DONE_M | RFC_DBELL_RFCPEIFG_LAST_FG_COMMAND_DONE_M))
    {
        /* IRQ_LAST_COMMAND_DONE received. */
        if ((pCmd = cmdQ.pCurrCmd))
        {
            /* No longer current command */
            cmdQ.pCurrCmd = NULL;

            /* Save RFCPEIFG for callback */
            pCmd->rfifg |= ((rfcpeifg & (RFC_DBELL_RFCPEIFG_LAST_COMMAND_DONE_M | RFC_DBELL_RFCPEIFG_LAST_FG_COMMAND_DONE_M | pCmd->bmEvent)));

            /* Move current command to done queue */
            HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFCPEIEN) &= ~(pCmd->bmEvent & RF_INTERNAL_CPE_IFG_MASK_32_BIT);
            HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFHWIEN) &= ~(pCmd->bmEvent >> RF_SHIFT_32_BITS);
            Q_push(&cmdQ.pDone, pCmd);

            /* Post SWI to handle registered callbacks and potentially start inactivity timer */
            Swi_or(Swi_handle(&swiFsm), Fsm_EventLastCommandDone);

            /* Check for background command (IEEE 15.4) */
            if((cmdQ.pCurrCmdBg) && (pCmd->pClient->clientConfig.pRfMode->rfMode == RF_MODE_IEEE_15_4))
            {
                if(pCmd->flags & RF_CMD_ABORTED_FLAG)
                {
                    cmdQ.pCurrCmdBg->flags |= RF_CMD_ABORTED_FLAG;
                    Q_push(&cmdQ.pDone, cmdQ.pCurrCmdBg);
                }
                else if(pCmd->flags & RF_CMD_STOPPED_FLAG)
                {
                    cmdQ.pCurrCmdBg->flags |= RF_CMD_STOPPED_FLAG;
                    Q_push(&cmdQ.pDone, cmdQ.pCurrCmdBg);
                }
                else
                {
                    cmdQ.pCurrCmd = cmdQ.pCurrCmdBg;
                }

                cmdQ.pCurrCmdBg = NULL;
            }
        }
    }

    /* Handle additional interrupts here */
    pCmd = cmdQ.pCurrCmd;
    if ((!(rfcpeifg & Fsm_EventLastCommandDone)) && (!(rfcpeifg & RFC_DBELL_RFCPEIFG_LAST_FG_COMMAND_DONE_M)) && (rfcpeifg & (pCmd->bmEvent)))
    {
        cmdQ.pCurrCmdCb = pCmd;

        /* Save CPEIFG for callback */
        pCmd->rfifg |= (rfcpeifg & pCmd->bmEvent);

        /* Post SWI to handle registered callbacks */
        Swi_or(Swi_handle(&swiFsm), Fsm_EventCpeInt);
    }

    /* Dispatch pending command */
    if ((!cmdQ.pCurrCmd) && (pCmd = Q_peek(&cmdQ.pPend)))
    {
        RF_dispatchCmd(pCmd);
    }
}

/*
 *  Function: clkInactivityFxn
 *
 *  Clock callback due to inactivity timeout
 *
 */
static void clkInactivityFxn(RF_Object* pObj)
{
    /* Check to release RF power constraints and potentially signal to FSM that we can power down radio */
    if (cmdQ.nSeqPost == cmdQ.nSeqDone)
    {
        RF_releasePowerConstraint(RF_PowerConstraintCmdQ);
    }
}

/*
 *  Function: clkReqAccessFxn
 *
 *  Clock callback due to request access timeout
 *
 */
static void clkReqAccessFxn(RF_Object* pObj)
{
    RF_issueRadioFreeCb(RF_RADIOFREECB_REQACCESS_FLAG | RF_RADIOFREECB_PREEMPT_FLAG |
                        RF_RADIOFREECB_CMDREJECT_FLAG);
}

/*
 *  Function: syncCb
 *
 *  Callback used to post semaphore for runCmd() and pendCmd()
 *
 */
static void syncCb(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
{
    if (h->state.pCbSync != NULL)
    {
        ((RF_Callback)h->state.pCbSync)(h, ch, e);
    }

    if ((e & RF_EventLastCmdDone) ||
        (e & h->state.eventSync)  ||
        ((h->clientConfig.pRfMode->rfMode == RF_MODE_IEEE_15_4) && (e & RF_EventLastFGCmdDone)))
    {
        h->state.unpendCause = e;
        Semaphore_post(Semaphore_handle(&h->state.semSync));
    }
}

/*
 *  Function: defaultErrCb
 *
 *  Default error callback function
 *
 */
static void defaultErrCb(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
{
    while(1);
}

/*-------------- RF powerup/powerdown FSM functions ---------------*/

/*
 *  Function: FsmDispatch
 *
 *  Affect FSM state change
 *
 */
static void FsmDispatch(RF_Object* pObj, Fsm_Event const e)
{
    pFsmState(pObj, e);
}


/*
 *  Function: FsmNextState
 *
 *   Set target state transition
 *
 */
static void FsmNextState(Fsm_StateFxn pTarget)
{
    pFsmState = pTarget;
}

/*
 *  Function: swiFxnFsm
 *
 *   The SWI handler for FSM events
 *
 */
static void swiFxnFsm(UArg a0, UArg a1)
{
    FsmDispatch(pCurrClient, (Fsm_Event)Swi_getTrigger());
}

/*
 *  Function: clkPowerUpFxn
 *
 *  Clock callback called upon powerup
 *
 */
static void clkPowerUpFxn(xdc_UArg a)
{
    if (pFsmState == fsmActiveState)
    {
        /* Set power constraint and post HWI for next command */
        RF_setPowerConstraint(RF_PowerConstraintCmdQ);
        Hwi_post(INT_RFC_CPE_0);
    }
    else
    {
        /* Trigger FSM SWI to get radio wakeup going */
        Swi_or(Swi_handle(&swiFsm), Fsm_EventWakeup);
    }
}

/*
 *  Function: hwiFxnCpe0PowerFsm
 *
 *  RF CPE0 ISR during FSM powerup/powerdown
 *
 */
static void hwiFxnCpe0PowerFsm(UArg a0)
{
    /* Clear all IRQ flags in doorbell and then clear them */
    uint32_t rfcpeifg = RFCCpeIntGetAndClear();

    RF_Cmd* pCmd;
    if (pFsmState == fsmActiveState)
    {
        /* Change HWI handler */
        Hwi_setFunc(Hwi_handle(&hwiCpe0), hwiFxnCpe0Active, (UArg)NULL);

        /* Mark radio and client as being active */
        bRadioActive = true;
        RF_setPowerConstraint(RF_PowerConstraintCmdQ);
        bPowerUpActive = false;

        /* If this is the first power-up for a RAT channel after a power cycle then
           send CMD_GET_FW_INFO command to update the value available RAT channels */
        if (cmdDirImm.availRatCh == RF_DEFAULT_AVAILRATCH_VAL)
        {
            rfc_CMD_GET_FW_INFO_t getFwInfoCmd;
            getFwInfoCmd.commandNo = CMD_GET_FW_INFO;
            RFCDoorbellSendTo((uint32_t)&getFwInfoCmd);
            cmdDirImm.availRatCh = getFwInfoCmd.availRatCh;
        }

        /* If there is a pending command  and no synth error, dispatch the cmd to CPE */
        if ((!cmdQ.pCurrCmd) && (pCmd = Q_peek(&cmdQ.pPend)))
        {
            if (!RF_checkCmdFsError())
            {
                RF_dispatchCmd(pCmd);
            }
        }
        else
        {
            Swi_or(Swi_handle(&swiFsm), Fsm_EventLastCommandDone);
        }
    }

    if ((rfcpeifg & IRQ_BOOT_DONE) || (rfcpeifg & IRQ_LAST_COMMAND_DONE))
    {
        Swi_or(Swi_handle(&swiFsm), Fsm_EventPowerStep);
    }
}

/*
 *  Function: hwiFxnCpe0ChangePhy
 *
 *  RF CPE0 ISR during Change PHY switching
 *
 */
static void hwiFxnCpe0ChangePhy(UArg a0)
{
    /* Clear all IRQ flags in doorbell and then clear them */
    uint32_t rfcpeifg = RFCCpeIntGetAndClear();

    if (rfcpeifg & IRQ_LAST_COMMAND_DONE)
    {
        Swi_or(Swi_handle(&swiFsm), Fsm_EventChangePhy2);
    }
}

/*-------------- Power management state functions ---------------*/
/*
 *  Function: applyRfCorePatch
 *
 *  Handles RF Core patching for CPE, MCE, RFE (if required) in setup state during power-up
 *
 */
static void applyRfCorePatch(bool mode)
{
    /* Patch CPE if relevant */
    if (pCurrClient->clientConfig.pRfMode->cpePatchFxn)
    {
        if (mode == RF_PHY_SWITCHING_MODE)
        {
            RFCCPEPatchReset();
        }
        pCurrClient->clientConfig.pRfMode->cpePatchFxn();
    }

    if ((pCurrClient->clientConfig.pRfMode->mcePatchFxn != 0) || (pCurrClient->clientConfig.pRfMode->rfePatchFxn != 0))
    {
        /* Wait for clocks to turn on*/
        while(!HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFACKIFG));
        HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFACKIFG) = 0;

        /* Patch MCE if relevant */
        if (pCurrClient->clientConfig.pRfMode->mcePatchFxn)
        {
            pCurrClient->clientConfig.pRfMode->mcePatchFxn();
        }

        /* Patch RFE if relevant */
        if (pCurrClient->clientConfig.pRfMode->rfePatchFxn)
        {
            pCurrClient->clientConfig.pRfMode->rfePatchFxn();
        }

        /* Turn off additional clocks */
        RFCDoorbellSendTo(CMDR_DIR_CMD_2BYTE(RF_CMD0, 0));
    }
}

/*
 *  Function: radioOpDoneCb
 *
 *  Handle callback to client for RF_EventLastCmdDone and issue radio free callback if required
 *
 */
static void radioOpDoneCb(RF_Cmd* pCmd)
{
    /* Radio command done */
    if (pCmd)
    {
        /* Update implicit radio state */
        updateImplicitState(pCmd);

        uint32_t tmp = pCmd->rfifg;
        pCmd->rfifg = 0;

        /* Issue callback, free container and dequeue */
        if (pCmd->pCb)
        {
            RF_EventMask cb_e;

            if(pCmd->pClient->clientConfig.pRfMode->rfMode == RF_MODE_IEEE_15_4)
            {
                cb_e = (RF_EventMask)(tmp);
            }
            else
            {
                cb_e = (RF_EventMask)(tmp | RF_EventLastCmdDone);
            }

            switch (pCmd->flags & RF_CMD_CANCEL_MASK)
            {
                case RF_CMD_ABORTED_FLAG:
                    cb_e = RF_EventCmdAborted;
                    break;
                case RF_CMD_STOPPED_FLAG:
                    cb_e = RF_EventCmdStopped;
                    break;
                case RF_CMD_CANCELLED_FLAG:
                    cb_e = RF_EventCmdCancelled;
                    break;
                default:
                    break;
            }

            if ((pCmd->flags & RF_CMD_PREEMPT_FLAG) && (pCmd->flags & RF_CMD_CANCEL_MASK))
            {
                cb_e |= RF_EventCmdPreempted;
            }
            pCmd->pCb(pCmd->pClient, pCmd->ch, cb_e);
        }

        uint32_t key = Hwi_disable();

        /* update num of radio command done */
        cmdQ.nSeqDone = (cmdQ.nSeqDone+1) & N_CMD_MODMASK;

        /* commmand completed reset command flags  */
        cmdQ.pDone->flags = 0;

        /* command completed, free command queue container */
        Q_pop(&cmdQ.pDone);

        Hwi_restore(key);

        /* Check if there are any more pending commands */
        if (cmdQ.nSeqDone == cmdQ.nSeqPost)
        {
            /* issue radio free callback after pre-emption if required */
            uint8_t tmp = RF_RADIOFREECB_PREEMPT_FLAG | RF_RADIOFREECB_CMDREJECT_FLAG;
            if (pCmd->pClient->state.bYielded)
            {
                tmp |= RF_RADIOFREECB_REQACCESS_FLAG;
            }
            RF_issueRadioFreeCb(tmp);

            /* Identify which is the other client */
            RF_Object *pOtherClient = RF_Sch.clientHnd[0];
            if (pCmd->pClient == RF_Sch.clientHnd[0])
            {
                pOtherClient = RF_Sch.clientHnd[1];
            }

            /* Check bYielded flag and inactivity time for this client */
            uint32_t inactivityTime = 0;
            if (!(pCmd->pClient->state.bYielded))
            {
                inactivityTime = pCmd->pClient->clientConfig.nInactivityTimeout;
            }
            /* Clear bYielded flag for this client */
            pCmd->pClient->state.bYielded = false;

            /* Check bYielded flag and inactivity time for the other client */
            if (pOtherClient != NULL)
            {
                if (pOtherClient->state.bYielded)
                {
                    /* Clear bYielded flag for the other clients */
                    pOtherClient->state.bYielded = false;
                }
                else
                {
                    inactivityTime = MAX(inactivityTime, pOtherClient->clientConfig.nInactivityTimeout);
                }
            }

            /* This was last posted command for client */
            if (inactivityTime == 0)
            {
                /* We can powerdown immediately */
                RF_releasePowerConstraint(RF_PowerConstraintCmdQ);
            }
            else
            {
                if (inactivityTime != BIOS_WAIT_FOREVER)
                {
                    /* Start inactivity timer */
                    uint32_t timeout = MAX(inactivityTime/Clock_tickPeriod, 1);
                    Clock_setTimeout(Clock_handle(&pCmd->pClient->state.clkInactivity), timeout);
                    Clock_start(Clock_handle(&pCmd->pClient->state.clkInactivity));
                }
            }
        }
    }
}

/*
 *  Function: fsmPowerUpState
 *  RF state machine function during power up state
 *  Input:  pObj - pointer to RF object
 *          e    - state machine event
 *  Return: none
 */
static void fsmPowerUpState(RF_Object *pObj, Fsm_Event e)
{
    /* Note: pObj is NULL in this state */
    if (e & Fsm_EventLastCommandDone)
    {
        /* issue radio operation done callback */
        RF_Cmd *pCmd = Q_peek(&cmdQ.pDone);
        radioOpDoneCb(pCmd);

        /* We've handled this event now */
        e &= ~Fsm_EventLastCommandDone;
        if ((pCmd = Q_peek(&cmdQ.pDone)))
        {
            /* Trigger self if there are more commands in callback queue */
            Swi_or(Swi_handle(&swiFsm), (e | Fsm_EventLastCommandDone));
        }
        else if ((e & Fsm_EventWakeup) || (Q_peek(&cmdQ.pPend)))
        {
            /* If there is pending cmd, just wake up */
            /* Device will put to sleep if the pending cmd is future cmd */
            Clock_stop(Clock_handle(&clkPowerUp));

            /* trigger power up */
            Swi_or(Swi_handle(&swiFsm), Fsm_EventWakeup);
        }
    }
    else if(e & Fsm_EventWakeup)
    {
        /* Disable system going to IDLE mode */
        bDisableFlashInIdleConstraint = true;
        Power_setConstraint(PowerCC26XX_NEED_FLASH_IN_IDLE);

        powerDurCalc.nRtc1 = AONRTCCurrent64BitValueGet();

        /* Set current client from first command in command queue */
        RF_Cmd* pNextCmd;
        if ((pNextCmd = Q_peek(&cmdQ.pPend)))
        {
            RF_Object* pNextClient = pNextCmd->pClient;
            if ((pCurrClient) && (pCurrClient != pNextClient))
            {
                /* do client switch callback */
                if ((pNextClient->clientConfig.pClientEventCb) && (pNextClient->clientConfig.nClientEventMask & RF_ClientEventSwitchClientEntered))
                {
                    RF_ClientCallback pClientEventCb = (RF_ClientCallback)pNextClient->clientConfig.pClientEventCb;
                    pClientEventCb(pNextClient, RF_ClientEventSwitchClientEntered, NULL);
                }
            }

            pCurrClient = pNextClient;
        }

        /* Keep track of mode */
        HWREG(PRCM_BASE + PRCM_O_RFCMODESEL) = pCurrClient->clientConfig.pRfMode->rfMode;
        if (HWREG(PRCM_BASE + PRCM_O_RFCMODESEL) != pCurrClient->clientConfig.pRfMode->rfMode)
        {
            /* Unsupported mode, do error callback */
            if (pCurrClient->clientConfig.pErrCb)
            {
                RF_Callback pErrCb = (RF_Callback)pCurrClient->clientConfig.pErrCb;
                pErrCb(pCurrClient, RF_ERROR_INVALID_RFMODE, RF_EventError);
            }
        }

        /* Register power dependency and constraints - i.e. power up RF. */
        Power_setConstraint(PowerCC26XX_SB_DISALLOW);
        Power_setDependency(PowerCC26XX_DOMAIN_RFCORE);

        /* Start power-up sequence */
        bPowerUpActive = true;

        /* Set the XOSC_HF dependency only for the first power up from a cold state if relevant  */
        if((pCurrClient->clientConfig.bPowerUpXOSC == false) || bStart == 0)
        {
            Power_setDependency(PowerCC26XX_XOSC_HF);
        }

        /* Turn on additional clocks on boot */
        if ((pCurrClient->clientConfig.pRfMode->mcePatchFxn != 0) || (pCurrClient->clientConfig.pRfMode->rfePatchFxn != 0))
        {
            HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFACKIFG) = 0;
            HWREG(RFC_DBELL_BASE+RFC_DBELL_O_CMDR) = CMDR_DIR_CMD_2BYTE(RF_CMD0, RFC_PWR_PWMCLKEN_MDMRAM | RFC_PWR_PWMCLKEN_RFERAM);
        }

        /* Turn on clock to RF core */
        RFCClockEnable();

        /* Map all radio interrupts to CPE0 and enable some sources */
        HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFCPEISL) = 0;

        HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFCPEIEN) = RFC_DBELL_RFCPEIEN_LAST_COMMAND_DONE_M |
                                                       RFC_DBELL_RFCPEIEN_BOOT_DONE_M |
                                                       RFC_DBELL_RFCPEIEN_LAST_FG_COMMAND_DONE_M;

        /* Next FSM state */
        FsmNextState(&fsmSetupState);

        /* Enable interrupts and let BOOT_DONE interrupt kick things off */
        Hwi_enableInterrupt(INT_RFC_CPE_0);
    }
}

/*
 *  Function: fsmSetupState
 *  RF state machine function during setup state
 *  Input:  pObj - pointer to RF object
 *          e    - state machine event
 *  Return: none
 */
static void fsmSetupState(RF_Object *pObj, Fsm_Event e)
{
    if (e & Fsm_EventPowerStep)
    {
        /* Apply RF Core patch (if required) */
        applyRfCorePatch(RF_PHY_BOOTUP_MODE);

        /* Initialize bus request */
        HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFACKIFG) = 0;
        HWREG(RFC_DBELL_BASE+RFC_DBELL_O_CMDR) = CMDR_DIR_CMD_1BYTE(CMD_BUS_REQUEST, 1);

        /* set VCOLDO reference */
        RFCAdi3VcoLdoVoltageMode(true);

        /* Next state: fsmActiveState */
        FsmNextState(&fsmActiveState);

        /* Setup RAT_SYNC command to follow SETUP command */
        opRatSync.start.commandNo                = CMD_SYNC_START_RAT;
        opRatSync.start.status                   = IDLE;
        opRatSync.start.startTrigger.triggerType = TRIG_NOW;
        opRatSync.start.pNextOp                  = NULL;
        opRatSync.start.condition.rule           = COND_NEVER;

        /* Setup SETUP command */
        RF_RadioSetup* pOpSetup         = pObj->clientConfig.pOpSetup;
        pOpSetup->prop.status           = IDLE;

        /* For power saving - analogCfgMode is set to 0 only for the first power up */
        if ((bStart == 0) || (pObj->clientConfig.bUpdateSetup))
        {
            if ((pObj->clientConfig.pOpSetup->commandId.commandNo == CMD_RADIO_SETUP) ||
                (pObj->clientConfig.pOpSetup->commandId.commandNo == CMD_BLE5_RADIO_SETUP))
            {
                /* BLE mode. BLE 5.0 shares same offset for config.analogCfgMode with BLE radio setup. */
                pOpSetup->common.config.analogCfgMode = RF_SETUP_ANALOGCFG_UPDATE;
            }
            else
            {
                /* PROP mode */
                pOpSetup->prop.config.analogCfgMode = RF_SETUP_ANALOGCFG_UPDATE;
            }

            pObj->clientConfig.bUpdateSetup = false;

            /* Check Override RTrim vs FCFG RTrim */
            RFCRTrim((rfc_radioOp_t*)pObj->clientConfig.pOpSetup);
        }

        /* Set RF Trim values */
        rfTrim_t rfTrim;
        RFCRfTrimRead((rfc_radioOp_t*)pObj->clientConfig.pOpSetup, (rfTrim_t*)&rfTrim);
        RFCRfTrimSet((rfTrim_t*)&rfTrim);

        RF_Op* tmp                      = (RF_Op*)&pOpSetup->prop;
        while ((tmp->pNextOp) && (tmp->pNextOp->commandNo != CMD_SYNC_START_RAT) &&
               (tmp->pNextOp->commandNo != CMD_FS) &&
               (tmp->pNextOp->commandNo != CMD_FS_OFF))
        {
            /* trace to the end of client queue */
            tmp = tmp->pNextOp;
        }
        tmp->pNextOp                    = (RF_Op*)&opRatSync.start;
        tmp->condition.rule             = COND_ALWAYS;

        /* Setup FS command to follow SETUP command */
        rfc_CMD_FS_t* pOpFs;
        RF_Op* pOpFirstPend = Q_peek(&cmdQ.pPend)->pOp;
        if ((pOpFirstPend->commandNo == CMD_FS) || (pOpFirstPend->commandNo == CMD_FS_OFF))
        {
            /* First command is FS command so no need to chain an implicit FS command -> Reset nRtc1 */
            powerDurCalc.nRtc1 = 0;
        }
        else
        {
            if (pObj->state.mode_state.cmdFs.commandNo)
            {
                /* Chain in the implicit FS command */
                pOpFs                           = &pObj->state.mode_state.cmdFs;
                pOpFs->status                   = IDLE;
                pOpFs->pNextOp                  = NULL;
                pOpFs->startTrigger.triggerType = TRIG_NOW;
                pOpFs->condition.rule           = COND_NEVER;
                opRatSync.start.pNextOp         = (RF_Op*)pOpFs;
                opRatSync.start.condition.rule  = COND_ALWAYS;
            }
        }

        /* power up the frequency synthesizer */
        pObj->clientConfig.pOpSetup->common.config.bNoFsPowerUp = 0;

        /* Make sure BUS_REQUEST is done */
        while(!HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFACKIFG));
        HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFACKIFG) = 0;

        /* Run the XOSC_HF switching if the pre-notify function setup the power
           constraint PowerCC26XX_DISALLOW_XOSC_HF_SWITCHING */
        if (bXOSCSwitchConstraint)
        {
            /* wait untill the XOSC_HF is stable */
            while(!PowerCC26XX_isStableXOSC_HF());
            /* run the XOSC_HF switch */
            PowerCC26XX_switchXOSC_HF();
        }
        else if (OSCClockSourceGet(OSC_SRC_CLK_HF) != OSC_XOSC_HF)
        {
            tmp->condition.rule = COND_NEVER;
            FsmNextState(&fsmXOSCState);
        }

        /* Send setup chain */
        HWREG(RFC_DBELL_BASE + RFC_DBELL_O_CMDR) = (uint32_t)&pObj->clientConfig.pOpSetup->commandId;
    }
}

/*
 *  Function: fsmXOSCState
 *  RF state machine function during XOSC state
 *  Input:  pObj - pointer to RF object
 *          e    - state machine event
 *  Return: none
 */
static void fsmXOSCState(RF_Object *pObj, Fsm_Event e)
{
    if ((e & Fsm_EventPowerStep) || (e & Fsm_EventWakeup))
    {
        if(OSCClockSourceGet(OSC_SRC_CLK_HF) == OSC_XOSC_HF)
        {
            FsmNextState(&fsmActiveState);

            /* Do RAT sync */
            HWREG(RFC_DBELL_BASE + RFC_DBELL_O_CMDR) = (uint32_t)&opRatSync.start;
        }
        else
        {
            /* Clock source not yet switched to XOSC_HF -> schedule polling */
            uint32_t timeout = MAX(RF_XOSC_HF_SWITCH_CHECK_PERIOD/Clock_tickPeriod, 1);
            Clock_setTimeout(Clock_handle(&clkPowerUp), timeout);
            Clock_start(Clock_handle(&clkPowerUp));
        }
    }
}

/*
 *  Function: fsmActiveState
 *  RF state machine function during active state
 *  Input:  pObj - pointer to RF object
 *          e    - state machine event
 *  Return: none
 */
static void fsmActiveState(RF_Object *pObj, Fsm_Event e)
{
    RF_Cmd* pCmd;
    uint32_t rtcValTmp1, rtcValTmp2;

    if (e & Fsm_EventCpeInt)
    {
        uint32_t key = Hwi_disable();
        pCmd = Q_peek(&cmdQ.pCurrCmdCb);
        uint32_t tmp = pCmd->rfifg;

        if (!(tmp & RF_EventLastCmdDone) && pCmd)
        {
            pCmd->rfifg &= (~tmp);
            Hwi_restore(key);
            if (pCmd->pCb && tmp)
            {
                pCmd->pCb(pCmd->pClient, pCmd->ch, (RF_EventMask)tmp);
            }
        }
        else
        {
            Hwi_restore(key);
        }

        /* We've handled this event now */
        e &= ~Fsm_EventCpeInt;
    }
    else if (e & Fsm_EventPowerStep)
    {
        /* Coming from powerup states */

        /* release disable IDLE since power up is now complete */
        if (bDisableFlashInIdleConstraint)
        {
            Power_releaseConstraint(PowerCC26XX_NEED_FLASH_IN_IDLE);
            bDisableFlashInIdleConstraint = false;
        }

        uint32_t key = Hwi_disable();

        /* For power saving, set analogCfgMode to 0x2D */
        if ((pObj->clientConfig.pOpSetup->commandId.commandNo == CMD_RADIO_SETUP) ||
            (pObj->clientConfig.pOpSetup->commandId.commandNo == CMD_BLE5_RADIO_SETUP))
        {
            /* BLE mode */
            pObj->clientConfig.pOpSetup->common.config.analogCfgMode = RF_SETUP_ANALOGCFG_NOUPDATE;
        }
        else
        {
            /* PROP mode */
            pObj->clientConfig.pOpSetup->prop.config.analogCfgMode = RF_SETUP_ANALOGCFG_NOUPDATE;
        }

        /* update power up duration if coming from the clkPowerUpFxn (nRtc not zero) */
        if (powerDurCalc.nRtc1 && (OSCClockSourceGet(OSC_SRC_CLK_LF) != OSC_RCOSC_HF))
        {
            rtcValTmp1  = (uint32_t)powerDurCalc.nRtc1;
            rtcValTmp2  = AONRTCCurrent64BitValueGet();
            if (rtcValTmp2 > rtcValTmp1)
            {
                pObj->clientConfig.nPowerUpDuration = (rtcValTmp2 - rtcValTmp1) >> RF_RTC_CONV_TO_US_SHIFT;
            }
            else
            {
                pObj->clientConfig.nPowerUpDuration = (0xFFFFFFFF + rtcValTmp2 - rtcValTmp1) >> RF_RTC_CONV_TO_US_SHIFT;
            }

            /* add margin to the power up duration to account for the TIRTOS/Drivers overhead */
            pObj->clientConfig.nPowerUpDuration += pObj->clientConfig.nPowerUpDurationMargin;

            /* power up duration should be within certain upper and lower bounds */
            if ((pObj->clientConfig.nPowerUpDuration > RF_DEFAULT_POWER_UP_TIME) ||
                (pObj->clientConfig.nPowerUpDuration < RF_DEFAULT_MIN_POWER_UP_TIME))
            {
                pObj->clientConfig.nPowerUpDuration = RF_DEFAULT_POWER_UP_TIME;
            }
        }

        Hwi_restore(key);

        /* Check the status of the CMD_FS, if it was sent (chained) to the setup command sent
           during Fsm_EventChangePhy1 and return error callback to the client
           client can either resend the CMD_FS or ignore the error as per Errata on PG2.1
           As this is a recoverable event do not call default err callback : while(1) */
        if (RF_checkCmdFsError())
        {
            RF_Callback pErrCb = (RF_Callback)pObj->clientConfig.pErrCb;
            pErrCb(pObj, RF_ERROR_CMDFS_SYNTH_PROG, RF_EventError);

            /* Check if there is pending command */
            if (Q_peek(&cmdQ.pPend))
            {
                /* Make sure pending command gets issued by issuing CPE0 IRQ */
                Hwi_post(INT_RFC_CPE_0);
            }
            else
            {
                /* no pending command */
                e |= Fsm_EventLastCommandDone;
            }
        }

        /* issue power up callback if relevant */
        if (pObj->clientConfig.pPowerCb)
        {
            RF_Callback pPowerCb = (RF_Callback)pObj->clientConfig.pPowerCb;
            pPowerCb(pObj, 0, RF_EventPowerUp);
        }

        /* We've handled this event now */
        e &= ~Fsm_EventPowerStep;
    }
    else if (e & Fsm_EventLastCommandDone)
    {
        /* issue radio operation done callback */
        pCmd = Q_peek(&cmdQ.pDone);
        radioOpDoneCb(pCmd);

        /* We've handled this event now */
        e &= ~Fsm_EventLastCommandDone;
        if ((pCmd = Q_peek(&cmdQ.pDone)))
        {
            /* Trigger self if there are more commands in callback queue */
            e |= Fsm_EventLastCommandDone;
        }
    }
    else if (e & Fsm_EventInitChangePhy)
    {
        RF_Cmd* pNextCmd;
        uint32_t key = Hwi_disable();
        if ((pNextCmd = Q_peek(&cmdQ.pPend)))
        {
            if (pCurrClient)
            {
                /* Stop inactivity clock of current client if running */
                Clock_stop(Clock_handle(&pCurrClient->state.clkInactivity));
            }

            /* switch the current client */
            pCurrClient = pNextCmd->pClient;
            Hwi_restore(key);

            /* measure switching time */
            powerDurCalc.nRtc2 = AONRTCCurrent64BitValueGet();

            /* change to fsmChangePhyState */
            FsmNextState(&fsmChangePhyState);
            Swi_or(Swi_handle(&swiFsm), Fsm_EventChangePhy1);
        }

        /* Clear flag, otherwise may cause problem later on when power down */
        bYieldPwrDnCancel = false;

        /* We've handled this event now */
        e &= ~Fsm_EventInitChangePhy;
    }
    else if (bYieldPwrDnCancel && (e & Fsm_EventPowerDown))
    {
        bYieldPwrDnCancel = false;
        e &= ~Fsm_EventPowerDown;
    }
    else if (cmdQ.pCurrCmd && (e & Fsm_EventPowerDown))
    {
        e &= ~Fsm_EventPowerDown;
    }
    else if (e & Fsm_EventPowerDown)
    {
        /* Radio not active */
        bRadioActive = false;
        bPowerDownActive = true;

        /* Next state: fsmPowerUpState */
        FsmNextState(&fsmPowerUpState);

        /* Execute power down sequence */
        RF_executePowerDown();

        /* Release constraints and dependencies, turning off RF core */
        Power_releaseConstraint(PowerCC26XX_SB_DISALLOW);
        Power_releaseDependency(PowerCC26XX_DOMAIN_RFCORE);
        if(pCurrClient->clientConfig.bPowerUpXOSC == false)
        {
            Power_releaseDependency(PowerCC26XX_XOSC_HF);
        }

        /* release constraint to switch XOSC_HF in rf driver if set during power up */
        if (bXOSCSwitchConstraint)
        {
            bXOSCSwitchConstraint = false;
            Power_releaseConstraint(PowerCC26XX_DISALLOW_XOSC_HF_SWITCHING);
        }

        /* Radio powered down */
        bPowerDownActive = false;
        bYieldPwrDnCalled = false;

        /* Start radio if required */
        if (bRestart)
        {
            bRestart = false;
            Swi_or(Swi_handle(&swiFsm), Fsm_EventWakeup);
        }

        /* issue radio available callback if RF_yield was called with no
           pending commands in the queue */
        uint8_t tmp;
        tmp = RF_RADIOFREECB_REQACCESS_FLAG;
        if (cmdQ.nSeqDone == cmdQ.nSeqPost)
        {
            tmp |= RF_RADIOFREECB_PREEMPT_FLAG | RF_RADIOFREECB_CMDREJECT_FLAG;
        }
        RF_issueRadioFreeCb(tmp);

        /* We've handled this event now */
        e &= ~Fsm_EventPowerDown;
    }
    else if (e & Fsm_EventHwInt)
    {
        /* RF_HW callback processing expect for RAT channel event */
        pCmd = Q_peek(&cmdQ.pCurrCmd);
        if (pCmd && pCmd->pCb)
        {
            pCmd->pCb(pCmd->pClient, pCmd->ch, RF_EventMdmSoft);
        }
        /* We've handled this event now */
        e &= ~Fsm_EventHwInt;
    }
    else if (e & Fsm_EventChangePhy2)
    {
        /* Check the status of the CMD_FS, if it was sent (chained) to the setup command sent
           during Fsm_EventChangePhy1 and return error callback to the client
           client can either resend the CMD_FS or ignore the error
           As this is a recoverable event do not call default err callback : while(1) */
        if (RF_checkCmdFsError())
        {
            RF_Callback pErrCb = (RF_Callback)pObj->clientConfig.pErrCb;
            pErrCb(pObj, RF_ERROR_CMDFS_SYNTH_PROG, RF_EventError);
        }

        /* Change HWI handler */
        Hwi_setFunc(Hwi_handle(&hwiCpe0), hwiFxnCpe0Active, (UArg)NULL);

        /* reset global variables */
        bPowerChangePhy = false;
        bRadioActive = true;

        /* For power saving, set analogCfgMode to 0x2D and clear bUpdateSetup flag */
        if ((pObj->clientConfig.pOpSetup->commandId.commandNo == CMD_RADIO_SETUP) ||
            (pObj->clientConfig.pOpSetup->commandId.commandNo == CMD_BLE5_RADIO_SETUP))
        {
            /* BLE mode */
            pObj->clientConfig.pOpSetup->common.config.analogCfgMode = RF_SETUP_ANALOGCFG_NOUPDATE;
        }
        else
        {
            /* PROP mode */
            pObj->clientConfig.pOpSetup->prop.config.analogCfgMode = RF_SETUP_ANALOGCFG_NOUPDATE;
        }
        pObj->clientConfig.bUpdateSetup = false;

        /* measure and update switching time */
        rtcValTmp1 = AONRTCCurrent64BitValueGet();
        uint8_t clientId = 1;

        if (pCurrClient == RF_Sch.clientHnd[0])
        {
            clientId = 0;
        }

        /* Only compute PHY switching time if nRtc2 is not zero */
        if (powerDurCalc.nRtc2)
        {
            if (rtcValTmp1 > powerDurCalc.nRtc2)
            {
                RF_Sch.phySwitchingTimeInUs[clientId] = (rtcValTmp1 - powerDurCalc.nRtc2) >> RF_RTC_CONV_TO_US_SHIFT;
            }
            else
            {
                RF_Sch.phySwitchingTimeInUs[clientId] = (0xFFFFFFFF + powerDurCalc.nRtc2 - rtcValTmp1) >> RF_RTC_CONV_TO_US_SHIFT;
            }
        }

        /* reset nRtc2 */
        powerDurCalc.nRtc2 = 0;

        /* post hwi to post the pending command */
        Hwi_post(INT_RFC_CPE_0);

        /* We have handled this event now */
        e &= ~Fsm_EventChangePhy2;
    }
    else if (e & Fsm_EventIeeeInt)
    {
        pCmd = Q_peek(&cmdQ.pPend);

        if((pCmd) && (pCmd->pClient->clientConfig.pRfMode->rfMode == RF_MODE_IEEE_15_4) && /* Next command is IEEE 15.4 */
           ((pCmd->flags & RF_CMD_BG_CMD_FLAG) == 0) &&                                    /* Next command is a foreground command */
           (cmdQ.pCurrCmd->flags & RF_CMD_BG_CMD_FLAG))                                    /* Current command is a background command */
        {
            /* May run user cb here */

            /* Check if previous command has started */
            while((cmdQ.pCurrCmd->pOp->status == IDLE) ||   /* Idle */
                  (cmdQ.pCurrCmd->pOp->status == PENDING)); /* Pending */

            /* Previous command has started, run foreground command */
            cmdQ.pCurrCmdBg = cmdQ.pCurrCmd;
            cmdQ.pCurrCmd = NULL;

            Hwi_post(INT_RFC_CPE_0);
        }

        e &= ~Fsm_EventIeeeInt;
    }
    else
    {
        /* No other FSM event has any effect */
    }

    /* Call self again if there are outstanding events to be processed */
    if (e)
    {
        Swi_or(Swi_handle(&swiFsm), e);
    }
}

/*
 *  Function: fsmChangePhyState
 *  RF state machine function during PHY change state
 *  Input:  pObj - pointer to RF object
 *          e    - state machine event
 *  Return: none
 */
static void fsmChangePhyState(RF_Object *pObj, Fsm_Event e)
{
    if (e & Fsm_EventChangePhy1)
    {
        /* set tracking global variables */
        bRadioActive = false;
        bPowerChangePhy = true;

        /* do client switch callback */
        if ((pCurrClient->clientConfig.pClientEventCb) && (pCurrClient->clientConfig.nClientEventMask & RF_ClientEventSwitchClientEntered))
        {
            RF_ClientCallback pClientEventCb = (RF_ClientCallback)pCurrClient->clientConfig.pClientEventCb;
            pClientEventCb(pCurrClient, RF_ClientEventSwitchClientEntered, NULL);
        }

        /* Change HWI handler */
        Hwi_setFunc(Hwi_handle(&hwiCpe0), hwiFxnCpe0ChangePhy, (UArg)NULL);

        /* change the next state */
        FsmNextState(&fsmActiveState);

        /* Re-enable MCE and RFE clocks */
        if ((pCurrClient->clientConfig.pRfMode->mcePatchFxn != 0) || (pCurrClient->clientConfig.pRfMode->rfePatchFxn != 0))
        {
            HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFACKIFG) = 0;
            HWREG(RFC_DBELL_BASE+RFC_DBELL_O_CMDR) = CMDR_DIR_CMD_2BYTE(RF_CMD0, RFC_PWR_PWMCLKEN_MDMRAM | RFC_PWR_PWMCLKEN_RFERAM);
        }

        /* Apply new RF Core patch */
        applyRfCorePatch(RF_PHY_SWITCHING_MODE);

        /* Initialize bus request */
        HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFACKIFG) = 0;
        HWREG(RFC_DBELL_BASE+RFC_DBELL_O_CMDR) = CMDR_DIR_CMD_1BYTE(CMD_BUS_REQUEST, 1);

        /* Setup SETUP command */
        RF_RadioSetup* pOpSetup         = pObj->clientConfig.pOpSetup;
        pOpSetup->prop.status           = IDLE;

        /* Need to write analog configuration when PHY is changed */
        if ((pObj->clientConfig.pOpSetup->commandId.commandNo == CMD_RADIO_SETUP) ||
            (pObj->clientConfig.pOpSetup->commandId.commandNo == CMD_BLE5_RADIO_SETUP))
        {
            /* BLE mode */
            pOpSetup->common.config.analogCfgMode = RF_SETUP_ANALOGCFG_UPDATE;
        }
        else
        {
            /* PROP mode */
            pOpSetup->prop.config.analogCfgMode = RF_SETUP_ANALOGCFG_UPDATE;
        }
        pObj->clientConfig.bUpdateSetup = true;

        RF_Op* tmp                      = (RF_Op*)&pOpSetup->prop;
        while ((tmp->pNextOp) && (tmp->pNextOp->commandNo != CMD_SYNC_START_RAT) &&
               (tmp->pNextOp->commandNo != CMD_FS) &&
               (tmp->pNextOp->commandNo != CMD_FS_OFF))
        {
            tmp = tmp->pNextOp;
        }
        tmp->pNextOp                    = NULL;
        tmp->condition.rule             = COND_NEVER;

        /* Setup FS command to follow SETUP command */
        rfc_CMD_FS_t* pOpFs;
        RF_Op* pOpFirstPend = Q_peek(&cmdQ.pPend)->pOp;
        if ((pOpFirstPend->commandNo == CMD_FS) || (pOpFirstPend->commandNo == CMD_FS_OFF))
        {
            /* First command is FS command so no need to chain an implicit FS command -> reset nRtc2 */
            powerDurCalc.nRtc2 = 0;
        }
        else
        {
            if (pObj->state.mode_state.cmdFs.commandNo)
            {
                /* Chain in the implicit FS command */
                pOpFs                            = &pObj->state.mode_state.cmdFs;
                pOpFs->status                    = IDLE;
                pOpFs->pNextOp                   = NULL;
                pOpFs->startTrigger.triggerType  = TRIG_NOW;
                pOpFs->condition.rule            = COND_NEVER;
                tmp->pNextOp                     = (RF_Op*)pOpFs;
                tmp->condition.rule              = COND_ALWAYS;
            }
        }

        /* Set RF Trim values */
        rfTrim_t rfTrim;
        RFCRfTrimRead((rfc_radioOp_t*)pObj->clientConfig.pOpSetup, (rfTrim_t*)&rfTrim);
        RFCRfTrimSet((rfTrim_t*)&rfTrim);

        /* Check Override RTrim vs FCFG RTrim */
        RFCRTrim((rfc_radioOp_t*)pObj->clientConfig.pOpSetup);

        /* Make sure BUS_REQUEST is done */
        while(!HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFACKIFG));
        HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFACKIFG) = 0;

        /* Send chain */
        HWREG(RFC_DBELL_BASE + RFC_DBELL_O_CMDR) = (uint32_t)&pObj->clientConfig.pOpSetup->commandId;

        /* We've handled this event now */
        e &= ~Fsm_EventChangePhy1;
    }
}

/*-------------- Initialization & helper functions ---------------*/

/*
 *  Function: rf_init
 *  Initialize RF driver
 *  Input:  none
 *  Return: none
 */
static void rf_init()
{
    union {
        Hwi_Params hp;
        Swi_Params sp;
    } params;

    /* Power init */
    Power_init();

    /* Enable output RTC clock for Radio Timer Synchronization */
    HWREG(AON_RTC_BASE + AON_RTC_O_CTL) |= AON_RTC_CTL_RTC_UPD_EN_M;

    /* Initialize SWIs and HWIs */
    Swi_Params_init(&params.sp);
    params.sp.priority = RFCC26XX_hwAttrs.swiCpe0Priority;
    Swi_construct(&swiFsm, swiFxnFsm, &params.sp, NULL);

    Hwi_Params_init(&params.hp);
    params.hp.enableInt = false;
    params.hp.priority = RFCC26XX_hwAttrs.hwiCpe0Priority;
    Hwi_construct(&hwiCpe0, INT_RFC_CPE_0, hwiFxnCpe0PowerFsm, &params.hp, NULL);

    Swi_Params_init(&params.sp);
    params.sp.priority = RFCC26XX_hwAttrs.swiHwPriority;
    Swi_construct(&swiHw, swiFxnHw, &params.sp, NULL);

    Hwi_Params_init(&params.hp);
    params.hp.enableInt = true;
    params.hp.priority = RFCC26XX_hwAttrs.hwiHwPriority;
    Hwi_construct(&hwiHw, INT_RFC_HW_COMB, hwiFxnHw, &params.hp, NULL);

    /* Initialize clock object used as power-up trigger */
    Clock_construct(&clkPowerUp, &clkPowerUpFxn, 0, NULL);

    /* Set FSM state to power up */
    FsmNextState(fsmPowerUpState);

    /* Subscribe to wakeup notification from the Power driver */
    Power_registerNotify(&RF_wakeupNotifyObj, PowerCC26XX_AWAKE_STANDBY,
                         (Power_NotifyFxn) RF_wakeupNotifyFxn, (uintptr_t) NULL);

    /* Initialize global variables */
    RF_CmdDirImm* pCmdDirImm = &cmdDirImm;
    pCmdDirImm->availRatCh = RF_DEFAULT_AVAILRATCH_VAL;
    pCmdDirImm->flags = 0;
    bStart = false;
    bPowerUpActive = false;
    bPowerDownActive = false;
    bYieldPwrDnCalled = false;
    bYieldPwrDnCancel = false;
    bPowerChangePhy = false;
    powerDurCalc.nRtc1 = 0;
    powerDurCalc.errTolValInUs = RF_DEFAULT_RAT_RTC_ERR_TOL_IN_US;
    RF_powerConstraint = 0;
    RF_Sch.phySwitchingTimeInUs[0] = RF_DEFAULT_PHY_SWITCHING_TIME;
    RF_Sch.phySwitchingTimeInUs[1] = RF_DEFAULT_PHY_SWITCHING_TIME;
    bXOSCSwitchConstraint = false;
    bDisableFlashInIdleConstraint = false;
}

/*
 *  Function: ratChanAlloc
 *
 *  Allocate RAT channel
 */
static int8_t ratChanAlloc(RF_CmdDirImm *pDirImmCmd)
{
    int8_t i;

    /* If RAT channels are available allocate one of the available RAT channels */
    if (pDirImmCmd->availRatCh != RF_DEFAULT_AVAILRATCH_VAL)
    {
        for (i = 0; i < 15; i++)
        {
            /* Update free RAT chan bitmap and create HW Event for allocated channel */
            if (pDirImmCmd->availRatCh & (1 << i))
            {
                pDirImmCmd->availRatCh &= ~(1<<i);
                HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFHWIEN) |= (1<<(i+RFC_DBELL_RFHWIFG_RATCH0_BITN));

                /* Set RF power constraint */
                RF_setPowerConstraint(RF_PowerConstraintRat);
                return i;
            }
        }
    }

    return (int8_t)RF_ALLOC_ERROR;
}

/*
 *  Function: ratChanFree
 *
 *  Free RAT channel
 */
static void ratChanFree(int8_t ratChannelNum, bool checkPowerDown)
{
    RF_CmdDirImm* pCmd = &cmdDirImm;

    uint32_t key = Hwi_disable();
    if (!((pCmd->flags & RF_RAT_RPT_CPT_FLAG) && (ratChannelNum == pCmd->rptCptRatChNum)))
    {
        pCmd->availRatCh |= (1 << ratChannelNum);
        HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFHWIEN) &= ~(1 << (ratChannelNum + RFC_DBELL_RFHWIFG_RATCH0_BITN));
        if ((!HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFHWIEN)) && checkPowerDown)
        {
            RF_releasePowerConstraint(RF_PowerConstraintRat);
        }
    }

    Hwi_restore(key);
}

/*
 *  Function: flushPendCmd
 *
 *  Trace through the pending queue and flush command from particular queue
 *  used by abortCmd()
 */
static void flushPendCmd(RF_Handle h, RF_Cmd *pCmdPend, bool bFlushAll)
{
    if (bFlushAll == true)
    {
        while (pCmdPend && (pCmdPend->pClient == h))
        {
            cmdQ.pPend = pCmdPend->pNext;
            pCmdPend->flags |= RF_CMD_CANCELLED_FLAG;
            Q_push(&cmdQ.pDone, pCmdPend);
            pCmdPend = cmdQ.pPend;
        }
    }

    while (pCmdPend)
    {
        if (pCmdPend->pNext && (pCmdPend->pNext->pClient == h))
        {
            RF_Cmd* pTemp = (RF_Cmd*)pCmdPend->pNext;
            pCmdPend->pNext = pTemp->pNext;
            pTemp->flags |= RF_CMD_CANCELLED_FLAG;
            Q_push(&cmdQ.pDone, pTemp);
        }
        else
        {
            pCmdPend = pCmdPend->pNext;
        }
    }
}

/*
 *  Function: abortCmd
 *
 *  Process cancel commands.
 *  used by RF_cancelCmd, RF_flushCmd API.
 */
static RF_Stat abortCmd(RF_Handle h, RF_CmdHandle ch, uint8_t type)
{
    /* Assert */
    Assert_isTrue((h != NULL), NULL);

    /* initialize local variables */
    RF_Cmd* pCmdPend = Q_peek(&cmdQ.pPend);
    RF_Cmd* pCmd;
    bool bGraceful = type & RF_ABORT_CANCEL_SINGLE_CMD;
    RF_Stat status = RF_StatInvalidParamsError;

    /* Check if current command */
    uint32_t key = Hwi_disable();
    if (ch == RF_CMDHANDLE_FLUSH_ALL)
    {
        if (cmdQ.pCurrCmd && (cmdQ.pCurrCmd->pClient == h))
        {
            ch = cmdQ.pCurrCmd->ch;
        }
        else
        {
            pCmd = Q_first(h, &cmdQ.pPend);
            if (pCmd)
            {
                ch = pCmd->ch;
            }
            else
            {
                Hwi_restore(key);
                return status;
            }
        }
    }
    pCmd = getCmd(ch);
    if (pCmd && (pCmd == cmdQ.pCurrCmd))
    {
        /* Flag that operation has been aborted. */
        pCmd->flags |= (1 << bGraceful);

        RFCDoorbellSendTo((bGraceful) ? CMDR_DIR_CMD(CMD_STOP) : CMDR_DIR_CMD(CMD_ABORT));

        /* Remove all cmds belonging to h */
        if (type & RF_ABORT_CANCEL_MULTIPLE_CMD)
        {
            pCmdPend = cmdQ.pPend;
            flushPendCmd(h, pCmdPend, true);
        }

        Hwi_restore(key);

        status = RF_StatSuccess;
    }
    else if (pCmd)
    {
        if (pCmdPend == pCmd)
        {
            /* cancel the first pending command */
            Clock_stop(Clock_handle(&clkPowerUp));
            cmdQ.pPend = cmdQ.pPend->pNext;
            pCmdPend->flags |= RF_CMD_CANCELLED_FLAG;
            Q_push(&cmdQ.pDone, pCmdPend);

            /* Remove all cmds belonging to h */
            if (type & RF_ABORT_CANCEL_MULTIPLE_CMD)
            {
                pCmdPend = cmdQ.pPend;
                flushPendCmd(h, pCmdPend, true);
            }
            Hwi_restore(key);

            status = RF_StatSuccess;
        }
        else
        {
            /* search for the command to be cancelled in the pending queue */
            while (pCmdPend)
            {
                if (pCmdPend->pNext == pCmd)
                {
                    Clock_stop(Clock_handle(&clkPowerUp));
                    RF_Cmd* pTemp = (RF_Cmd*)pCmdPend->pNext;

                    pCmdPend->pNext = pTemp->pNext;
                    pTemp->flags |= RF_CMD_CANCELLED_FLAG;
                    Q_push(&cmdQ.pDone, pTemp);

                    if (type & RF_ABORT_CANCEL_MULTIPLE_CMD)
                    {
                        flushPendCmd(h, pCmdPend, false);
                    }
                    Hwi_restore(key);
                    status = RF_StatSuccess;
                    break;
                }
                else
                {
                    pCmdPend = (RF_Cmd*)pCmdPend->pNext;
                }
            }
            if (status != RF_StatSuccess)
            {
                Hwi_restore(key);
            }
        }

        /* If power up active is set, the last command done will be posted when power setup is complete */
        if(!bPowerUpActive)
        {
            Swi_or(Swi_handle(&swiFsm), Fsm_EventLastCommandDone);
        }
    }
    else
    {
        Hwi_restore(key);
    }

    return(status);
}

/*-------------- API functions ---------------*/
/*
 * ======== RF_open ========
 * Open an RF handle
 */
RF_Handle RF_open(RF_Object *pObj, RF_Mode *pRfMode, RF_RadioSetup *pOpSetup, RF_Params *params)
{
    Assert_isTrue((pObj != NULL), NULL);

    /* Initialize driver on first call */
    if (!bInitialized)
    {
        rf_init();
        bInitialized = true;
    }

    if (pRfMode != NULL)
    {
        // Check whether RF mode is valid
        uint32_t rfcModeHwOpt = HWREG(PRCM_BASE + PRCM_O_RFCMODEHWOPT);
        if (rfcModeHwOpt & (1 << pRfMode->rfMode))
        {
            pObj->clientConfig.pRfMode = pRfMode;
        }
        else
        {
            return NULL;
        }
    }
    else
    {
        return NULL;
    }

    if (pOpSetup != NULL)
    {
        pObj->clientConfig.pOpSetup = pOpSetup;
    }
    else
    {
        return NULL;
    }

    /* Check whether RF driver is accepting more clients */
    uint32_t key = Hwi_disable();
    if (nClients < N_MAX_CLIENTS)
    {
        /* save the new RF_Handle */
        RF_Sch.clientHnd[nClients] = pObj;
        nClients++;
    }
    else
    {
        Hwi_restore(key);
        return NULL;
    }
    Hwi_restore(key);

    /* Populate default params if not provided */
    RF_Params rfParams;
    if (params == NULL)
    {
        RF_Params_init(&rfParams);
        params = &rfParams;
    }

    /* Initialize RF_Object config */
    pObj->clientConfig.nInactivityTimeout     = params->nInactivityTimeout;
    pObj->clientConfig.nPowerUpDuration       = params->nPowerUpDuration;
    pObj->clientConfig.bPowerUpXOSC           = params->bPowerUpXOSC;
    pObj->clientConfig.pPowerCb               = (void *)params->pPowerCb;
    pObj->clientConfig.pClientEventCb         = (void *)params->pClientEventCb;
    pObj->clientConfig.nClientEventMask       = params->nClientEventMask;
    pObj->clientConfig.bUpdateSetup           = true;
    pObj->clientConfig.nPowerUpDurationMargin = params->nPowerUpDurationMargin;

    if (params->pErrCb)
    {
        pObj->clientConfig.pErrCb           = (void *)params->pErrCb;
    }

    /* Initialize client state & variables */
    memset(&pObj->state, 0, sizeof(pObj->state));

    /* Initialize client semaphore and clock object */
    union {
        Semaphore_Params sp;
        Clock_Params cp;
    } paramsUnion;

    Semaphore_Params_init(&paramsUnion.sp);
    paramsUnion.sp.mode = Semaphore_Mode_BINARY;
    Semaphore_construct(&pObj->state.semSync, 0, &paramsUnion.sp);
    Clock_Params_init(&paramsUnion.cp);
    paramsUnion.cp.arg = (UArg)pObj;
    Clock_construct(&pObj->state.clkInactivity, (Clock_FuncPtr)clkInactivityFxn, 0, &paramsUnion.cp);

    Clock_Params_init(&paramsUnion.cp);
    paramsUnion.cp.arg = (UArg)pObj;
    Clock_construct(&pObj->state.clkReqAccess, (Clock_FuncPtr)clkReqAccessFxn, 0, &paramsUnion.cp);

    return pObj;
}

/*
 * ======== RF_close ========
 * Close an RF handle
 */
void RF_close(RF_Handle h)
{
    Assert_isTrue((h != NULL), NULL);

    /* Wait for all issued commands to finish */
    if(bInitialized)
    {
        if (cmdQ.nSeqPost != cmdQ.nSeqDone)
        {
            /* Trace the end of pending queue for client h */
            RF_Cmd *pCmd = Q_end(h, &cmdQ.pPend);
            if (!pCmd)
            {
                if (cmdQ.pCurrCmd && (cmdQ.pCurrCmd->pClient == h))
                {
                    /* no pending commands for client h
                       -> check if the current command is the last command */
                    pCmd = cmdQ.pCurrCmd;
                }
            }
            if (pCmd)
            {
                RF_pendCmd(h, pCmd->ch, (RF_EventLastCmdDone | RF_EventCmdAborted | RF_EventCmdStopped | RF_EventCmdCancelled));
            }
        }

        /* Check whether this is the last client */
        uint32_t key = Hwi_disable();
        if (nClients && (--nClients == 0))
        {
            Hwi_restore(key);
            bInitialized = false;
            pCurrClient = h;

            if (bRadioActive)
            {
                /* Power down radio */
                RF_releasePowerConstraint(RF_PowerConstraintCmdQ);
                RF_releasePowerConstraint(RF_PowerConstraintRat);

                /* Wait for powerdown and unregister module RTOS objects */
                Semaphore_pend(Semaphore_handle(&h->state.semSync), BIOS_WAIT_FOREVER);
            }

            /* unregister RTOS objects initalized during rf_init */
            Semaphore_destruct(&h->state.semSync);
            Clock_destruct(&h->state.clkInactivity);
            Clock_destruct(&h->state.clkReqAccess);
            Swi_destruct(&swiFsm);
            Hwi_destruct(&hwiCpe0);
            Swi_destruct(&swiHw);
            Hwi_destruct(&hwiHw);
            Clock_destruct(&clkPowerUp);

            /* reset current client handler */
            pCurrClient = NULL;

            /* unregister the wakeup notify callback */
            Power_unregisterNotify(&RF_wakeupNotifyObj);

            /* release XOSC_HF dependency if relevant */
            if (h->clientConfig.bPowerUpXOSC == true)
            {
                Power_releaseDependency(PowerCC26XX_XOSC_HF);
            }
        }
        else
        {
            Hwi_restore(key);

            /* If we're the current RF client, stop being it */
            if (pCurrClient==h)
            {
                pCurrClient = NULL;
            }

            /* Unregister client RTOS objects */
            Semaphore_destruct(&h->state.semSync);
            Clock_destruct(&h->state.clkInactivity);
            Clock_destruct(&h->state.clkReqAccess);
        }

        /* clear the RF_sch client handle */
        if (h == RF_Sch.clientHnd[0])
        {
            RF_Sch.clientHnd[0] = NULL;
        }
        else
        {
            RF_Sch.clientHnd[1] = NULL;
        }
    }
}

/*
 * ======== RF_getCurrentTime ========
 * Get current time in RAT ticks
 */
uint32_t RF_getCurrentTime(void)
{
    uint32_t key = Hwi_disable();
    if (bRadioActive)
    {
        /* Radio is active, directly read RAT count */
        uint32_t ratCnt;
        ratCnt = HWREG(RFC_RAT_BASE + RFC_RAT_O_RATCNT);
        Hwi_restore(key);
        return ratCnt;
    }
    else
    {
        /* Radio is off, read RTC and convert to RAT count */
        Hwi_restore(key);
        uint64_t nRtc;
        nRtc  = AONRTCCurrent64BitValueGet();

        /* Conservatively assume that we are just about to increment the RTC
           Scale with the 4 MHz that the RAT is running
           Add the RAT offset for RTC==0 */
        nRtc += RF_RTC_TICK_INC;
        nRtc *= RF_SCALE_RTC_TO_4MHZ;
        nRtc += ((uint64_t)opRatSync.start.rat0) << RF_SHIFT_32_BITS;
        return (uint32_t)(nRtc >> RF_SHIFT_32_BITS);
    }
}

/*
 * ======== RF_postCmd ========
 * Post radio command
 */
RF_CmdHandle RF_postCmd(RF_Handle h, RF_Op* pOp, RF_Priority ePri, RF_Callback pCb, RF_EventMask bmEvent)
{
    RF_Cmd* pCmd;
    RF_CmdHandle ch;

    /* Assert */
    Assert_isTrue((h != NULL), NULL);
    Assert_isTrue((pOp != NULL), NULL);


    uint32_t key = Hwi_disable();

    /* Try to allocate container and handle nSeqPost/nSeqDone overflow */
    if (((cmdQ.nSeqPost + N_CMD_MODMASK + 1 - cmdQ.nSeqDone) & N_CMD_MODMASK) < N_CMD_POOL)
    {
        /* Allocate RF_Op container */
        cmdQ.nSeqPost = (cmdQ.nSeqPost + 1) & N_CMD_MODMASK;
        ch = cmdQ.nSeqPost;

        pCmd = allocCmd(ch);
    }
    else
    {
        /* No more RF command containers available -> set status and return error code */
        Hwi_restore(key);
        return (RF_CmdHandle)RF_ALLOC_ERROR;
    }

    /* Stop inactivity clock if running */
    Clock_stop(Clock_handle(&h->state.clkInactivity));

    /* Populate RF_Op container */
    pCmd->pOp = pOp;
    pCmd->ePri = ePri;
    pCmd->pCb = pCb;
    pCmd->ch = ch;
    pCmd->pClient = h;
    pCmd->bmEvent = bmEvent & ~RF_INTERNAL_IFG_MASK;
    pCmd->flags = 0;

    if (cmdQ.pPend)
    {
        /* Submit to pending command queue */
        if (Q_push(&cmdQ.pPend, pCmd) == false)
        {
            cmdQ.nSeqPost = (N_CMD_MODMASK + cmdQ.nSeqPost) & N_CMD_MODMASK;
            Hwi_restore(key);
            return (RF_CmdHandle)RF_ALLOC_ERROR;
        }

        h->state.chLastPosted = ch;
        pCmd->flags |= RF_CMD_ALLOC_FLAG;
        /* Reset yield flag */
        h->state.bYielded = false;
        Hwi_restore(key);
        return ch;
    }

    /* Submit to pending command queue */
    if (Q_push(&cmdQ.pPend, pCmd) == false)
    {
        cmdQ.nSeqPost = (N_CMD_MODMASK + cmdQ.nSeqPost) & N_CMD_MODMASK;
        Hwi_restore(key);
        return (RF_CmdHandle)RF_ALLOC_ERROR;
    }

    h->state.chLastPosted = ch;
    pCmd->flags |= RF_CMD_ALLOC_FLAG;

    /* Reset yield flag */
    h->state.bYielded = false;
    if (bYieldPwrDnCalled && bRadioActive)
    {
        bYieldPwrDnCancel = true;
        bYieldPwrDnCalled = false;
    }
    Hwi_restore(key);

    /* Trigger dispatcher HWI if there is no running command */
    if (!cmdQ.pCurrCmd)
    {
        if (bRadioActive)
        {
            /* Set RF power constraint */
            RF_setPowerConstraint(RF_PowerConstraintCmdQ);
            Hwi_post(INT_RFC_CPE_0);
        }
        else if (!bPowerUpActive)
        {
            /* Wakeup radio. FSM will trigger dispatch again when powered up */
            key = Hwi_disable();
            if (bPowerDownActive)
            {
                bRestart = true;
                Hwi_restore(key);
            }
            else if (dispatchOpWhen(pCmd->pOp))
            {
                Clock_start(Clock_handle(&clkPowerUp));
                Hwi_restore(key);
            }
            else
            {
                Hwi_restore(key);
                Swi_or(Swi_handle(&swiFsm), Fsm_EventWakeup);
            }
        }
    }
    return ch;
}

/*
 *  ==================== RF_scheduleCmd ============================
 *  Process request to schedule new command from a particular client
 */
RF_CmdHandle RF_scheduleCmd(RF_Handle h, RF_Op* pOp, RF_ScheduleCmdParams *pSchParams, RF_Callback pCb, RF_EventMask bmEvent)
{
    /* local variable declaration */
    RF_Cmd* pCmd;
    RF_CmdHandle ch;
    RF_Handle h2;

    /* Assert */
    Assert_isTrue((h != NULL), NULL);
    Assert_isTrue((pOp != NULL), NULL);

    uint32_t key = Hwi_disable();

    /* Assign h2 to client not requesting the command
       h is the client requesting the new command */
    if (h == RF_Sch.clientHnd[0])
    {
        h2 = RF_Sch.clientHnd[1];
    }
    else
    {
        h2 = RF_Sch.clientHnd[0];
    }

    /* scheduler results for new command
         1. new commmand is rejected -> return RF_ALLOC_ERROR
         2. new command is added to the queue (preemption or append)
         3. new command is inserted in the pending queue, but not at the top
         4. new command is inserted at the top of the pending queue

       After all the cases, if the new command is added/inserted to top of the
       pending queue, post new command if there is no current running command.
    */
    RF_ScheduleCmdStatus schStatus;
    pCmd = RF_schCmdRunInsertPreempt(h, h2, pOp, pSchParams, &schStatus);

    /* Case 1 */
    if (!pCmd)
    {
        RF_Sch.issueRadioFreeCbFlags |= RF_RADIOFREECB_CMDREJECT_FLAG;
        RF_Sch.clientHndRadioFreeCb = h;
        Hwi_restore(key);
        if (schStatus == RF_ScheduleCmdSchError)
        {
            return RF_SCHEDULE_CMD_ERROR;
        }
        else
        {
            return RF_ALLOC_ERROR;
        }
    }

    /* Stop inactivity clock if running */
    Clock_stop(Clock_handle(&h->state.clkInactivity));

    /* Populate RF_Op container */
    cmdQ.nSeqPost = pCmd->ch;
    ch = pCmd->ch;
    pCmd->pOp = pOp;
    pCmd->ePri = pSchParams->priority;
    pCmd->pCb = pCb;
    pCmd->ch = ch;
    pCmd->pClient = h;
    pCmd->bmEvent = bmEvent & ~RF_INTERNAL_IFG_MASK;
    pCmd->flags = 0;

    if((pCmd->pClient->clientConfig.pRfMode->rfMode == RF_MODE_IEEE_15_4) &&
       (pSchParams->bIeeeBgCmd == 1))
    {
        pCmd->flags |= RF_CMD_BG_CMD_FLAG;
    }

    /* Case 2 */
    Hwi_restore(key);

    /* Send the RF_flushCmd outside critical section */
    if (RF_Sch.cmdInsertFlags == RF_SCH_CMD_PREEMPT)
    {
        RF_flushCmd(RF_Sch.preemptCmd.client,
                    RF_Sch.preemptCmd.cancelStartCmdHandle,
                    RF_Sch.preemptCmd.cancelType);
    }
    key = Hwi_disable();

    if ((RF_Sch.cmdInsertFlags == 0) || (RF_Sch.cmdInsertFlags == RF_SCH_CMD_PREEMPT))
    {
        if (cmdQ.pPend)
        {
            /* Submit to pending command queue */
            if (Q_push(&cmdQ.pPend, pCmd) == false)
            {
                cmdQ.nSeqPost = (N_CMD_MODMASK + cmdQ.nSeqPost) & N_CMD_MODMASK;
                Hwi_restore(key);
                return (RF_CmdHandle)RF_ALLOC_ERROR;
            }

            h->state.chLastPosted = ch;
            pCmd->flags |= RF_CMD_ALLOC_FLAG;
            h->state.bYielded = false;
            Hwi_restore(key);
            return ch;
        }

        /* Submit to pending command queue */
        if (Q_push(&cmdQ.pPend, pCmd) == false)
        {
            cmdQ.nSeqPost = (N_CMD_MODMASK + cmdQ.nSeqPost) & N_CMD_MODMASK;
            Hwi_restore(key);
            return (RF_CmdHandle)RF_ALLOC_ERROR;
        }

        h->state.chLastPosted = ch;
        pCmd->flags |= RF_CMD_ALLOC_FLAG;
        h->state.bYielded = false;
        if (bYieldPwrDnCalled && bRadioActive)
        {
            bYieldPwrDnCancel = true;
            bYieldPwrDnCalled = false;
        }
        Hwi_restore(key);
    }

    /* Case 3 */
    if (RF_Sch.cmdInsertFlags == RF_SCH_CMD_INSERT_QUEUE_LATER)
    {
        pCmd->flags |= RF_CMD_ALLOC_FLAG;
        h->state.bYielded = false;
        Hwi_restore(key);
        return pCmd->ch;
    }

    /* Case 4 */
    if (RF_Sch.cmdInsertFlags == RF_SCH_CMD_INSERT_QUEUE_TOP)
    {
        Clock_stop(Clock_handle(&clkPowerUp));
        pCmd->flags |= RF_CMD_ALLOC_FLAG;
        h->state.bYielded = false;

        /* if there is already current command running -> return */
        if (cmdQ.pCurrCmd)
        {
            Hwi_restore(key);
            return pCmd->ch;
        }
        else
        {
            Hwi_restore(key);
        }
    }

    /* Trigger dispatcher HWI if there is no running command */
    if (!cmdQ.pCurrCmd)
    {
        cmdQ.pCurrCmdBg = NULL;

        if (bRadioActive)
        {
            /* Set RF power constraints */
            RF_setPowerConstraint(RF_PowerConstraintCmdQ);

            Hwi_post(INT_RFC_CPE_0);
        }
        else if (!bPowerUpActive)
        {
            /* Wakeup radio. FSM will trigger dispatch again when powered up */
            key = Hwi_disable();
            if (bPowerDownActive)
            {
                bRestart = true;
                Hwi_restore(key);
            }
            else if (dispatchOpWhen(pCmd->pOp))
            {
                Clock_start(Clock_handle(&clkPowerUp));
                Hwi_restore(key);
            }
            else if (!bPowerChangePhy)
            {
                Hwi_restore(key);
                Swi_or(Swi_handle(&swiFsm), Fsm_EventWakeup);
            }
            else
            {
                /* do nothing */
                Hwi_restore(key);
            }
        }
    }
    else if((cmdQ.pCurrCmd->pClient->clientConfig.pRfMode->rfMode == RF_MODE_IEEE_15_4) &&
            (pCmd->pClient->clientConfig.pRfMode->rfMode == RF_MODE_IEEE_15_4) &&
            (pSchParams->bIeeeBgCmd == 0))
    {
        /* IEEE 15.4 mode only */
        /* Run Fsm_EventIeeeInt only if new cmd is a foreground command */
        Swi_or(Swi_handle(&swiFsm), Fsm_EventIeeeInt);
    }

    return ch;
}

RF_EventMask RF_pendCmd(RF_Handle h, RF_CmdHandle ch, RF_EventMask bmEvent)
{
    /* Assert */
    Assert_isTrue((h != NULL), NULL);
    Assert_isTrue((ch >= 0), NULL);


    /* Check whether command has finished, if not override command callback */
    uint32_t key = Swi_disable();

    RF_Cmd* pCmd = getCmd(ch);

    /* check if null */
    if (!pCmd || !(pCmd->flags & RF_CMD_ALLOC_FLAG))
    {
        Swi_restore(key);
        return RF_EventLastCmdDone;
    }

    /* check if aborted */
    if (pCmd->flags & (RF_CMD_ABORTED_FLAG | RF_CMD_STOPPED_FLAG | RF_CMD_CANCELLED_FLAG | RF_CMD_PREEMPT_FLAG))
    {
        Swi_restore(key);
        while(pCmd->flags & (RF_CMD_ABORTED_FLAG | RF_CMD_STOPPED_FLAG | RF_CMD_CANCELLED_FLAG | RF_CMD_PREEMPT_FLAG));
        return RF_EventLastCmdDone;
    }

    /* Command has still not finished, override callback with one that posts to semaphore */
    if (pCmd->pCb != syncCb)
    {
        h->state.pCbSync = (void*)pCmd->pCb;
        pCmd->pCb = syncCb;
    }

    h->state.eventSync = bmEvent;
    Swi_restore(key);

    /* Wait for semaphore */
    Semaphore_pend(Semaphore_handle(&h->state.semSync), BIOS_WAIT_FOREVER);

    /* Clear user cb */
    h->state.pCbSync = NULL;

    /* Return command event */
    return h->state.unpendCause;
}

/*
 *  ======== RF_runCmd ========
 *  Run to completion a posted command
 */
RF_EventMask RF_runCmd(RF_Handle h, RF_Op* pOp, RF_Priority ePri, RF_Callback pCb, RF_EventMask bmEvent)
{
    if (pCb == NULL)
    {
        pCb = syncCb;
    }

    /* post requested command to RF_postCmd */
    RF_CmdHandle ch = RF_postCmd(h, pOp, ePri, pCb, bmEvent);

    if (ch<0)
    {
        return RF_EventCmdError;
    }

    /* wait for the command to complete using RF_pendCmd */
    return RF_pendCmd(h, ch, (RF_EventLastCmdDone | RF_EventCmdAborted | RF_EventCmdStopped | RF_EventCmdCancelled));
}

/*
 *  ======== RF_runScheduleCmd ========
 *  Run to completion a scheduled command
 */
RF_EventMask RF_runScheduleCmd(RF_Handle h, RF_Op* pOp, RF_ScheduleCmdParams *pSchParams, RF_Callback pCb, RF_EventMask bmEvent)
{
    if (pCb == NULL)
    {
        pCb = syncCb;
    }

    /* post requested command to RF_postCmd */
    RF_CmdHandle ch = RF_scheduleCmd(h, pOp, pSchParams, pCb, bmEvent);

    if (ch<0)
    {
        return RF_EventCmdError;
    }

    /* wait for the command to complete using RF_pendCmd */
    return RF_pendCmd(h, ch, (RF_EventLastCmdDone | RF_EventCmdAborted | RF_EventCmdStopped | RF_EventCmdCancelled));
}

/*
 *  ======== RF_yieldCmd ========
 *  Release client access
 */
void RF_yield(RF_Handle h)
{
    /* Assert */
    Assert_isTrue((h != NULL), NULL);

    uint32_t key = Hwi_disable();

    /* set bRatSync to indicate rat/stc sync at every power down, equivalent to
       setting powerDurCalc.errTolValInUs = 0 */
    bRatSync = true;

    /* Stop ongoing request access and issue callback if the radio is off */
    Clock_stop((Clock_handle(&h->state.clkReqAccess)));
    if (!bRadioActive && RF_Sch.issueRadioFreeCbFlags && (cmdQ.nSeqDone == cmdQ.nSeqPost))
    {
        Hwi_restore(key);
        RF_issueRadioFreeCb(RF_RADIOFREECB_REQACCESS_FLAG |
                            RF_RADIOFREECB_PREEMPT_FLAG | RF_RADIOFREECB_CMDREJECT_FLAG);
        key = Hwi_disable();
    }

    if(cmdQ.nSeqPost == cmdQ.nSeqDone)
    {
        /* All commands are done -> stop inactivity timer, decrement active
           client count and potentially signal powerdown to FSM */
        Clock_stop(Clock_handle(&h->state.clkInactivity));
        RF_releasePowerConstraint(RF_PowerConstraintCmdQ);
        if (bRadioActive)
        {
            bYieldPwrDnCalled = true;
        }
        Hwi_restore(key);
        return;
    }

    /* There are still client commands that haven't finished
       -> set flag to indicate immediate powerdown when last command is done */
    h->state.bYielded = true;

    /* Restore HWI */
    Hwi_restore(key);
}

/*
 *  ======== RF_cancelCmd ========
 *  Cancel single radio command
 */
RF_Stat RF_cancelCmd(RF_Handle h, RF_CmdHandle ch, uint8_t mode)
{
    return abortCmd(h, ch, mode);
}

/*
 *  ======== RF_flushCmd ========
 *  Cancel multiple radio commands from a client
 */
RF_Stat RF_flushCmd(RF_Handle h, RF_CmdHandle ch, uint8_t mode)
{
    return (abortCmd(h, ch, (mode | RF_ABORT_CANCEL_MULTIPLE_CMD)));
}

/*
 *  ======== RF_Params_init ========
 *  Initialize the RF_params to default value
 */
void RF_Params_init(RF_Params *params)
{
    /* Assert */
    Assert_isTrue((params != NULL), NULL);

    /* assign default values for RF_params */
    *params = RF_defaultParams;
}


static RF_Stat postDirImmCmd(uint32_t pCmdStruct, bool bStartRadio)
{
    uint32_t key = Hwi_disable();
    if (bRadioActive)
    {
        /* run command now */
        uint32_t cmdstaVal = RFCDoorbellSendTo(pCmdStruct);
        Hwi_restore(key);

        /* check return CMDSTA value and set return status */
        if ((cmdstaVal & RF_CMDSTA_REG_VAL_MASK) == CMDSTA_Done)
        {
            return RF_StatCmdDoneSuccess;
        }
        else
        {
            return RF_StatCmdDoneError;
        }
    }
    else
    {
        Hwi_restore(key);
        return RF_StatRadioInactiveError;
    }
}

/*
 *  ======== RF_runImmediateCmd ========
 *  Run immediate command
 */
RF_Stat RF_runImmediateCmd(RF_Handle h, uint32_t* pCmdStruct)
{
    /* Assert */
    Assert_isTrue((h != NULL), NULL);

    /* Return Error case if this is not the current client */
    if (h != pCurrClient)
    {
        return RF_StatInvalidParamsError;
    }

    /* Run command */
    return postDirImmCmd((uint32_t)pCmdStruct, false);
}

/*
 *  ======== RF_runDirectCmd ========
 *  Run direct command
 */
RF_Stat RF_runDirectCmd(RF_Handle h, uint32_t cmd)
{
    /* Assert */
    Assert_isTrue((h != NULL), NULL);

    /* Return Error case if this is not the current client */
    if (h != pCurrClient)
    {
        return RF_StatInvalidParamsError;
    }

    /* run command now */
    return postDirImmCmd(cmd, false);
}

/*
 *  ======== RF_getRssi ========
 *  Get RSSI value
 */
int8_t RF_getRssi(RF_Handle h)
{
    /* return error if this is not the current client or radio is off */
    if ((h!= pCurrClient) || (!bRadioActive))
    {
        return RF_GET_RSSI_ERROR_VAL;
    }

    uint32_t key = Hwi_disable();
    /* sent CMD_GET_RSSI as direct command to CPE */
    uint32_t cmdsta = RFCDoorbellSendTo(CMDR_DIR_CMD(CMD_GET_RSSI));
    Hwi_restore(key);

    /* check CMDSTA value to return valid RSSI value or return error */
    if ((cmdsta & RF_CMDSTA_REG_VAL_MASK) == CMDSTA_Done)
    {
        return (int8_t)((cmdsta >> RF_SHIFT_16_BITS) & RF_CMDSTA_REG_VAL_MASK);
    }
    else
    {
        return (int8_t)RF_GET_RSSI_ERROR_VAL;
    }
}

/*
 *  ======== RF_getInfo ========
 *  Get RF driver info
 */
RF_Stat RF_getInfo(RF_Handle h, RF_InfoType type, RF_InfoVal *pValue)
{
    int8_t kk = 0;
    RF_ScheduleMapElement *pScheduleMap;
    RF_Stat status = RF_StatSuccess;

    uint32_t key = Hwi_disable();
    switch(type)
    {
        case RF_GET_CURR_CMD:
            /* Get currently running command */
            if (cmdQ.pCurrCmd)
            {
                pValue->ch = cmdQ.pCurrCmd->ch;
            }
            else
            {
                status = RF_StatError;
            }
            break;
        case RF_GET_AVAIL_RAT_CH:
            /* Get available RAT channels */
            if (cmdDirImm.availRatCh != RF_DEFAULT_AVAILRATCH_VAL)
            {
                pValue->availRatCh = cmdDirImm.availRatCh;
            }
            else
            {
                status = RF_StatError;
            }
            break;
        case RF_GET_RADIO_STATE:
            /* Get current radio state */
            pValue->bRadioState = bRadioActive;
            break;
        case RF_GET_SCHEDULE_MAP:
            /* Get scheduler timing map */
            pScheduleMap = (RF_ScheduleMapElement *)pValue->pScheduleMap;
            memset(pScheduleMap, 0, sizeof(RF_ScheduleMapElement) * RF_NUM_SCHEDULE_MAP_ENTRIES);

            for (kk = 0; kk < RF_NUM_SCHEDULE_ACCESS_ENTRIES; kk++)
            {
                /* Copy access request info to schedule map */
                pScheduleMap[kk].pClient   = RF_Sch.clientHnd[kk];
                pScheduleMap[kk].priority  = RF_Sch.accReq[kk].priority;
                uint32_t startTime = RF_Sch.accReq[kk].startTime;
                pScheduleMap[kk].startTime = startTime;
                pScheduleMap[kk].endTime = startTime + RF_Sch.accReq[kk].duration;
            }

            /* Check if there is current command running */
            if (cmdQ.pCurrCmd)
            {
                /* Copy current command info to schedule map */
                pScheduleMap[kk].pClient   = cmdQ.pCurrCmd->pClient;
                pScheduleMap[kk].priority  = cmdQ.pCurrCmd->ePri;
                pScheduleMap[kk].startTime = cmdQ.pCurrCmd->startTime;
                pScheduleMap[kk].endTime   = cmdQ.pCurrCmd->endTime;
                kk++;
            }

            /* Check pending commands */
            RF_Cmd* pCmd;
            pCmd = cmdQ.pPend;
            /* Loop until end of command queue or number of entries exceed */
            while (pCmd)
            {
                if (kk < RF_NUM_SCHEDULE_MAP_ENTRIES)
                {
                    /* Copy pending command info to schedule map */
                    pScheduleMap[kk].pClient   = pCmd->pClient;
                    pScheduleMap[kk].priority  = pCmd->ePri;
                    pScheduleMap[kk].startTime = pCmd->startTime;
                    pScheduleMap[kk].endTime   = pCmd->endTime;
                    kk++;
                }
                else
                {
                    /* Number of entries exceeded, get out of loop */
                    break;
                }
                pCmd = pCmd->pNext;
            }
            break;
        default:
            status = RF_StatInvalidParamsError;
            break;
    }
    Hwi_restore(key);

    return status;
}

/*
 *  ======== RF_getCmdOp ========
 *  Get RF command
 */
RF_Op* RF_getCmdOp(RF_Handle h, RF_CmdHandle cmdHnd)
{
    RF_Cmd* pCmd = getCmd(cmdHnd);
    if (pCmd)
    {
        return (pCmd->pOp);
    }
    else
    {
        return NULL;
    }
}

/*
 *  ======== RF_ratCompare ========
 *  Set RAT compare
 */
int8_t RF_ratCompare(RF_Handle h, rfc_CMD_SET_RAT_CMP_t* pCmdStruct, \
                            uint32_t compareTime, RF_Callback pRatCb)
{
    if ((!bRadioActive) && (!bPowerChangePhy))
    {
        return RF_ALLOC_ERROR;
    }

    RF_CmdDirImm* pDirImmCmd = &cmdDirImm;

    pCmdStruct->commandNo = CMD_SET_RAT_CMP;
    pCmdStruct->compareTime = compareTime;

    /* check if any channel is available for allocation */
    int8_t ratCh = ratChanAlloc(pDirImmCmd);

    if (ratCh >= 0)
    {
        pCmdStruct->ratCh = ratCh;

        /* Send command to doorbell - only when radio is active or during PHY switching */
        uint32_t key = Hwi_disable();
        RF_Stat status = RF_StatCmdDoneError;

        /* run command now */
        uint32_t cmdstaVal = RFCDoorbellSendTo((uint32_t)pCmdStruct);
        Hwi_restore(key);

        /* check CMDSTA value and set return status */
        if ((cmdstaVal & RF_CMDSTA_REG_VAL_MASK) == CMDSTA_Done)
        {
            status = RF_StatCmdDoneSuccess;
        }
        else
        {
            status = RF_StatCmdDoneError;
        }

        /* free allocated rat channel if CMD_SET_RAT_CMP returns error */
        if ((status == RF_StatCmdDoneError) || (status == RF_StatRadioInactiveError))
        {
            ratChanFree(ratCh, true);
            ratCh = RF_ALLOC_ERROR;
        }
        else
        {
            /*command executed successfully -> set the RAT callback fxn and client */
            pDirImmCmd->pRatCb[ratCh - RF_RAT_CH_OFFSET] = pRatCb;
            pDirImmCmd->pClient[ratCh - RF_RAT_CH_OFFSET] = h;
        }
    }

    return ratCh;
}

/*
 *  ======== RF_ratCapture ========
 *  Set RAT capture
 */
int8_t RF_ratCapture(RF_Handle h, uint16_t config, RF_Callback pRatCb)
{
    if ((!bRadioActive) && (!bPowerChangePhy))
    {
        return RF_ALLOC_ERROR;
    }

    RF_CmdDirImm* pDirImmCmd = &cmdDirImm;

    int8_t ratCh = ratChanAlloc(pDirImmCmd);

    if (ratCh >= 0)
    {
        uint32_t key = Hwi_disable();
        config |= (ratCh << RF_SHIFT_8_BITS) & RF_RAT_CPT_CONFIG_RATCH_MASK;
        uint32_t cmdstaVal;
        /* run CMD_SET_RAT_CPT as direct command */
        cmdstaVal = RFCDoorbellSendTo((uint32_t)CMDR_DIR_CMD_2BYTE(CMD_SET_RAT_CPT, config));

        /* check CMDSTA value and set return status */
        if ((cmdstaVal & RF_CMDSTA_REG_VAL_MASK) == CMDSTA_Done)
        {
            if (config & RF_RAT_CPT_CONFIG_REP_MASK)
            {
                HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFHWIFG) &= ~(1 << (ratCh + RFC_DBELL_RFHWIFG_RATCH0_BITN));
                pDirImmCmd->rptCptRatChNum = ratCh;
                pDirImmCmd->flags |= RF_RAT_RPT_CPT_FLAG;
            }
            pDirImmCmd->pClient[ratCh - RF_RAT_CH_OFFSET] = h;
            pDirImmCmd->pRatCb[ratCh - RF_RAT_CH_OFFSET] = pRatCb;
        }
        else
        {
            /* free allocated rat channel if CMD_SET_RAT_CMP returns error */
            ratChanFree(ratCh, true);
            ratCh = RF_ALLOC_ERROR;
        }
        Hwi_restore(key);
    }
    return ratCh;
}

/*
 *  ======== RF_ratHwOutput ========
 *  Set RAT HW output
 */
RF_Stat RF_ratHwOutput(RF_Handle h, uint16_t config)
{
    return postDirImmCmd(((uint32_t)CMDR_DIR_CMD_2BYTE(CMD_SET_RAT_OUTPUT, config)), false);
}

/*
 *  ======== RF_ratDisableChannel ========
 *  Disable RAT channel
 */
RF_Stat RF_ratDisableChannel(RF_Handle h, int8_t ratChannelNum)
{
    RF_CmdDirImm* pCmd = &cmdDirImm;

    if ((!bRadioActive) && (!bPowerChangePhy))
    {
        return RF_StatRadioInactiveError;
    }

    if ((pCmd->flags & RF_RAT_RPT_CPT_FLAG) && (ratChannelNum == pCmd->rptCptRatChNum))
    {
        pCmd->flags &= ~RF_RAT_RPT_CPT_FLAG;
        pCmd->rptCptRatChNum = 0;
    }

    uint32_t key = Hwi_disable();
    /* disable Swi to delay a potential power down untill Swi_restore() */
    uint32_t keySwi = Swi_disable();

    /* free RAT channel at RF Driver layer */
    ratChanFree(ratChannelNum, true);
    /* send CMD_DISABLE_RAT_CH as direct command to CPE */
    uint32_t cmdStaVal = RFCDoorbellSendTo(CMDR_DIR_CMD_2BYTE(CMD_DISABLE_RAT_CH, (ratChannelNum << 8)));
    HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFHWIFG) &= ~(1 << (ratChannelNum + RFC_DBELL_RFHWIFG_RATCH0_BITN));

    /* restore Swi to allow power down if initiated */
    Swi_restore(keySwi);
    Hwi_restore(key);

    /* free allocated rat channel if CMD_SET_RAT_CMP returns error */
    if ((cmdStaVal & RF_CMDSTA_REG_VAL_MASK) == CMDSTA_Done)
    {
        return RF_StatCmdDoneSuccess;
    }
    else
    {
        return RF_StatCmdDoneError;
    }
}

/*
 *  ======== RF_control ========
 *  RF control
 */
RF_Stat RF_control(RF_Handle h, int8_t ctrl, void *args)
{
    RF_Stat ret = RF_StatError;

    uint32_t key = Hwi_disable();

    switch (ctrl)
    {
        case RF_CTRL_SET_INACTIVITY_TIMEOUT:
            h->clientConfig.nInactivityTimeout = *(uint32_t *)args;
            ret = RF_StatSuccess;
            break;

        case RF_CTRL_UPDATE_SETUP_CMD:
            /* Update setup command */
            h->clientConfig.bUpdateSetup = true;

            /* Since the next setup command will configure analog, need extra nPowerupDuration */
            h->clientConfig.nPowerUpDuration += RF_ANALOG_CFG_TIME_US;
            ret = RF_StatSuccess;
            break;

        case RF_CTRL_SET_POWERUP_DURATION_MARGIN:
            h->clientConfig.nPowerUpDurationMargin = *(uint32_t *)args;
            ret = RF_StatSuccess;
            break;

        case RF_CTRL_SET_RAT_RTC_ERR_TOL_VAL:
            powerDurCalc.errTolValInUs = *(uint32_t*)args;
            ret = RF_StatSuccess;
            break;

        case RF_CTRL_SET_POWER_MGMT:
            if (*(uint32_t*)args == 0)
            {
                /* Disable power management by setting RF power constraint */
                RF_setPowerConstraint(RF_PowerConstraintAll);
                ret = RF_StatSuccess;
            }
            else if (*(uint32_t*)args == 1)
            {
                /* Enable power management by releasing RF power constraint */
                RF_releasePowerConstraint(RF_PowerConstraintAll);
                ret = RF_StatSuccess;
            }
            else
            {
                ret = RF_StatInvalidParamsError;
            }
            break;

        default:
            ret = RF_StatInvalidParamsError;
            break;
    }

    Hwi_restore(key);

    return ret;
}

/*
 *  ======== RF_requestAccess ========
 *  RF request access
 */
RF_Stat RF_requestAccess(RF_Handle h, RF_AccessParams *pParams)
{
    /* Assert */
    Assert_isTrue((h != NULL), NULL);
    Assert_isTrue((pParams != NULL), NULL);

    uint32_t durationInUs = (pParams->duration  >> 2);

    /* check if RF_AccessParams are within range  */
    if ((durationInUs > RF_REQ_ACCESS_MAX_DUR_US) ||
        (pParams->priority != RF_PriorityHighest))
    {
        return RF_StatInvalidParamsError;
    }

    uint32_t key = Hwi_disable();
    uint8_t clientIdx = 0;
    if (h == RF_Sch.clientHnd[1])
    {
        clientIdx = 1;
    }

    /*  Check if, dedicated access request can be granted.
        If possible update the RF_Sch structure and start the timer (RTC) for the reqd duration.
        else, return RF_StatBusyError     */
    uint8_t reqAccChk = RF_schReqAccessCheck(h, RF_Sch.clientHnd[clientIdx ^ 0x1]);
    if (reqAccChk)
    {
        RF_Sch.accReq[clientIdx].duration = pParams->duration;
        RF_Sch.accReq[clientIdx].priority = pParams->priority;

        /* Start request access clock timer */
        uint32_t timeout = MAX(durationInUs/Clock_tickPeriod, 1);
        Clock_setTimeout(Clock_handle(&h->state.clkReqAccess), timeout);
        Clock_start(Clock_handle(&h->state.clkReqAccess));
        Hwi_restore(key);

        /* if there was preemption send RF_flushCmd */
        if (reqAccChk == 1)
        {
            RF_flushCmd(RF_Sch.preemptCmd.client,
                        RF_Sch.preemptCmd.cancelStartCmdHandle,
                        RF_Sch.preemptCmd.cancelType);
        }
        return RF_StatSuccess;
    }
    else
    {
        /* setup callback to issue radio available callback after ongoing req access is
           complete */
        RF_Sch.issueRadioFreeCbFlags |= RF_RADIOFREECB_REQACCESS_FLAG;
        RF_Sch.clientHndRadioFreeCb = h;
        Hwi_restore(key);
        return RF_StatBusyError;
    }
}
