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
/*
 *  ======== PowerCC26XX.c ========
 */

#include <xdc/std.h>
#include <stdbool.h>
#include <xdc/runtime/Assert.h>
#include <ti/sysbios/family/arm/m3/Hwi.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Swi.h>

#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26XX.h>

/* driverlib header files */
#ifdef DEVICE_FAMILY
    #undef DEVICE_FAMILY_PATH
    #define DEVICE_FAMILY_PATH(x) <ti/devices/DEVICE_FAMILY/x>
#else
    #error "You must define DEVICE_FAMILY at the project level as one of cc26x0, cc26x0r2, cc13x0, etc."
#endif

#include DEVICE_FAMILY_PATH(inc/hw_types.h)
#include DEVICE_FAMILY_PATH(inc/hw_prcm.h)
#include DEVICE_FAMILY_PATH(inc/hw_nvic.h)
#include DEVICE_FAMILY_PATH(inc/hw_aon_wuc.h)
#include DEVICE_FAMILY_PATH(inc/hw_aon_rtc.h)
#include DEVICE_FAMILY_PATH(inc/hw_memmap.h)
#include DEVICE_FAMILY_PATH(inc/hw_ccfg.h)
#include DEVICE_FAMILY_PATH(inc/hw_rfc_pwr.h)
#include DEVICE_FAMILY_PATH(driverlib/sys_ctrl.h)
#include DEVICE_FAMILY_PATH(driverlib/pwr_ctrl.h)
#include DEVICE_FAMILY_PATH(driverlib/prcm.h)
#include DEVICE_FAMILY_PATH(driverlib/aon_wuc.h)
#include DEVICE_FAMILY_PATH(driverlib/aon_ioc.h)
#include DEVICE_FAMILY_PATH(driverlib/aon_rtc.h)
#include DEVICE_FAMILY_PATH(driverlib/aon_event.h)
#include DEVICE_FAMILY_PATH(driverlib/aux_wuc.h)
#include DEVICE_FAMILY_PATH(driverlib/osc.h)
#include DEVICE_FAMILY_PATH(driverlib/cpu.h)
#include DEVICE_FAMILY_PATH(driverlib/vims.h)
#include DEVICE_FAMILY_PATH(driverlib/rfc.h)
#include DEVICE_FAMILY_PATH(driverlib/sys_ctrl.h)
#include DEVICE_FAMILY_PATH(driverlib/driverlib_release.h)
#include DEVICE_FAMILY_PATH(driverlib/setup.h)
#include DEVICE_FAMILY_PATH(driverlib/ccfgread.h)

static unsigned int configureXOSCHF(unsigned int action);
static unsigned int nopResourceHandler(unsigned int action);
static unsigned int configureRFCoreClocks(unsigned int action);
static void switchXOSCHFclockFunc(uintptr_t arg0);
static void disableLFClockQualifiers(uintptr_t arg);
static void emptyClockFunc(uintptr_t arg);
static int_fast16_t notify(uint_fast16_t eventType);

/* RCOSC calibration functions functions */
extern void PowerCC26XX_doCalibrate(void);
extern bool PowerCC26XX_initiateCalibration(void);
extern void PowerCC26XX_auxISR(uintptr_t arg);
extern void PowerCC26XX_RCOSC_clockFunc(uintptr_t arg);

/* Externs */
extern const PowerCC26XX_Config PowerCC26XX_config;

/* Module_State */
PowerCC26XX_ModuleState PowerCC26XX_module = {
    .notifyList = { NULL },         /* list of registered notifications    */
    .constraintMask = 0,            /* the constraint mask                 */
    .clockObj = { NULL },           /* Clock object for scheduling wakeups */
    .xoscClockObj = { NULL },       /* Clock object for XOSC_HF switching  */
    .lfClockObj = { NULL },         /* Clock object for LF clock check     */
    .calClockStruct = { NULL },     /* Clock object for RCOSC calibration  */
    .hwiStruct = { NULL },          /* hwi object for calibration          */
    .nDeltaFreqCurr = 0,            /* RCOSC calibration variable          */
    .nCtrimCurr = 0,                /* RCOSC calibration variable          */
    .nCtrimFractCurr = 0,           /* RCOSC calibration variable          */
    .nCtrimNew = 0,                 /* RCOSC calibration variable          */
    .nCtrimFractNew = 0,            /* RCOSC calibration variable          */
    .nRtrimNew = 0,                 /* RCOSC calibration variable          */
    .nRtrimCurr = 0,                /* RCOSC calibration variable          */
    .nDeltaFreqNew = 0,             /* RCOSC calibration variable          */
    .bRefine = false,               /* RCOSC calibration variable          */
    .state = Power_ACTIVE,          /* current transition state            */
    .xoscPending = FALSE,           /* is XOSC_HF activation in progress?  */
    .calLF = FALSE,                 /* calibrate RCOSC_LF?                 */
    .hwiState = 0,                  /* calibration AUX ISR state           */
    .busyCal = FALSE,               /* already busy calibrating            */
    .calStep = 1,                   /* current calibration step            */
    .firstLF = TRUE,                /* is this first LF calibration?       */
    .enablePolicy = FALSE,          /* default value is FALSE              */
    .initialized = FALSE,           /* whether Power_init has been called  */
    .constraintCounts = { 0, 0, 0, 0, 0, 0 },
    .resourceCounts = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    .resourceHandlers = {
      configureRFCoreClocks,
      configureXOSCHF,
      nopResourceHandler
    },                              /* special resource handler functions */
    .policyFxn = NULL               /* power policyFxn */
};

/* resource database */
const PowerCC26XX_ResourceRecord resourceDB[PowerCC26XX_NUMRESOURCES] = {
    { PowerCC26XX_PERIPH  | PowerCC26XX_DOMAIN_PERIPH, PRCM_PERIPH_TIMER0 },    /* PERIPH_GPT0 */
    { PowerCC26XX_PERIPH  | PowerCC26XX_DOMAIN_PERIPH, PRCM_PERIPH_TIMER1 },    /* PERIPH_GPT1 */
    { PowerCC26XX_PERIPH  | PowerCC26XX_DOMAIN_PERIPH, PRCM_PERIPH_TIMER2 },    /* PERIPH_GPT2 */
    { PowerCC26XX_PERIPH  | PowerCC26XX_DOMAIN_PERIPH, PRCM_PERIPH_TIMER3 },    /* PERIPH_GPT3 */
    { PowerCC26XX_PERIPH  | PowerCC26XX_DOMAIN_SERIAL, PRCM_PERIPH_SSI0 },      /* PERIPH_SSI0 */
    { PowerCC26XX_PERIPH  | PowerCC26XX_DOMAIN_PERIPH, PRCM_PERIPH_SSI1 },      /* PERIPH_SSI1 */
    { PowerCC26XX_PERIPH  | PowerCC26XX_DOMAIN_SERIAL, PRCM_PERIPH_UART0 },     /* PERIPH_UART0 */
    { PowerCC26XX_PERIPH  | PowerCC26XX_DOMAIN_SERIAL, PRCM_PERIPH_I2C0 },      /* PERIPH_I2C0 */
    { PowerCC26XX_PERIPH  | PowerCC26XX_DOMAIN_PERIPH, PRCM_PERIPH_TRNG },      /* PERIPH_TRNG */
    { PowerCC26XX_PERIPH  | PowerCC26XX_DOMAIN_PERIPH, PRCM_PERIPH_GPIO },      /* PERIPH_GPIO */
    { PowerCC26XX_PERIPH  | PowerCC26XX_DOMAIN_PERIPH, PRCM_PERIPH_UDMA },      /* PERIPH_UDMA */
    { PowerCC26XX_PERIPH  | PowerCC26XX_DOMAIN_PERIPH, PRCM_PERIPH_CRYPTO },    /* PERIPH_CRYPTO */
    { PowerCC26XX_PERIPH  | PowerCC26XX_DOMAIN_PERIPH, PRCM_PERIPH_I2S },       /* PERIPH_I2S */
    { PowerCC26XX_SPECIAL | PowerCC26XX_DOMAIN_RFCORE, 0 },                     /* PERIPH_RFCORE */
    { PowerCC26XX_SPECIAL | PowerCC26XX_NOPARENT, 1 },                          /* XOSC_HF */
    { PowerCC26XX_DOMAIN  | PowerCC26XX_NOPARENT, PRCM_DOMAIN_PERIPH },         /* DOMAIN_PERIPH */
    { PowerCC26XX_DOMAIN  | PowerCC26XX_NOPARENT, PRCM_DOMAIN_SERIAL },         /* DOMAIN_SERIAL */
    { PowerCC26XX_DOMAIN  | PowerCC26XX_NOPARENT, PRCM_DOMAIN_RFCORE },         /* DOMAIN_RFCORE */
    { PowerCC26XX_SPECIAL | PowerCC26XX_NOPARENT, 2 }                           /* DOMAIN_SYSBUS */
};


/* ****************** Power APIs ******************** */

/*
 *  ======== Power_disablePolicy ========
 *  Do not run the configured policy
 */
void Power_disablePolicy(void)
{
    PowerCC26XX_module.enablePolicy = FALSE;
}

/*
 *  ======== Power_enablePolicy ========
 *  Run the configured policy
 */
void Power_enablePolicy(void)
{
    PowerCC26XX_module.enablePolicy = TRUE;
}

/*
 *  ======== Power_getConstraintMask ========
 *  Get a bitmask indicating the constraints that have been registered with
 *  Power.
 */
uint_fast32_t Power_getConstraintMask(void)
{
    return (PowerCC26XX_module.constraintMask);
}

/*
 *  ======== Power_getDependencyCount ========
 *  Get the count of dependencies that are currently declared upon a resource.
 */
int_fast16_t Power_getDependencyCount(uint_fast16_t resourceId)
{
    /* Assert resourceId is valid */
    Assert_isTrue(resourceId < PowerCC26XX_NUMRESOURCES, NULL);

    return ((int_fast16_t)PowerCC26XX_module.resourceCounts[resourceId]);
}

/*
 *  ======== Power_getTransitionLatency ========
 *  Get the transition latency for a sleep state.  The latency is reported
 *  in units of microseconds.
 */
uint_fast32_t Power_getTransitionLatency(uint_fast16_t sleepState,
    uint_fast16_t type)
{
    uint32_t latency = 0;

    if (type == Power_RESUME) {
        if (sleepState == PowerCC26XX_STANDBY) {
            latency = PowerCC26XX_RESUMETIMESTANDBY;
        }
    }
    else {
        if (sleepState == PowerCC26XX_STANDBY) {
            latency = PowerCC26XX_TOTALTIMESTANDBY;
        }
    }

    return (latency);
}

/*
 *  ======== Power_getTransitionState ========
 *  Get the current sleep transition state.
 */
uint_fast16_t Power_getTransitionState(void)
{
    return (PowerCC26XX_module.state);
}

/*
 *  ======== Power_idleFunc ========
 *  Function needs to be plugged into the idle loop.
 *  It calls the configured policy function if the
 *  'enablePolicy' flag is set.
 */
void Power_idleFunc()
{
    if (PowerCC26XX_module.enablePolicy) {
        if (PowerCC26XX_module.policyFxn != NULL) {
            (*(PowerCC26XX_module.policyFxn))();
        }
    }
}

/*
 *  ======== Power_init ========
 */
int_fast16_t Power_init()
{
    Clock_Params clockParams;
    uint32_t ccfgLfClkSrc;
    uint32_t timeout;

    /* if this function has already been called, just return */
    if (PowerCC26XX_module.initialized) {
        return (Power_SOK);
    }

    /* set module state field 'initialized' to true */
    PowerCC26XX_module.initialized = TRUE;

    /* set the module state enablePolicy field */
    PowerCC26XX_module.enablePolicy = PowerCC26XX_config.enablePolicy;

    /* copy the Power policy function to module state */
    PowerCC26XX_module.policyFxn = PowerCC26XX_config.policyFxn;

    /* construct the Clock object for scheduling of wakeups */
    /* initiated and started by the power policy */
    Clock_Params_init(&clockParams);
    clockParams.period = 0;
    clockParams.startFlag = FALSE;
    clockParams.arg = 0;
    Clock_construct(&PowerCC26XX_module.clockObj, &emptyClockFunc,
        0, &clockParams);

    /* construct the Clock object for XOSC_HF switching */
    /* initiated and started by Power module when activating XOSC_HF */
    Clock_construct(&PowerCC26XX_module.xoscClockObj, &switchXOSCHFclockFunc,
        0, &clockParams);

    /* construct the Clock object for disabling LF clock quailifiers */
    /* one shot, auto start, first expires at 100 msec */
    Clock_construct(&PowerCC26XX_module.lfClockObj, &disableLFClockQualifiers,
        0, &clockParams);

    (*(PowerCC26XX_config.calibrateFxn))(PowerCC26XX_SETUP_CALIBRATE);

    DRIVERLIB_ASSERT_CURR_RELEASE();

    /* read the LF clock source from CCFG */
    ccfgLfClkSrc = CCFGRead_SCLK_LF_OPTION();

    /* check if should calibrate RCOSC_LF */
    if (PowerCC26XX_config.calibrateRCOSC_LF) {
        /* verify RCOSC_LF is the LF clock source */
        if (ccfgLfClkSrc == CCFGREAD_SCLK_LF_OPTION_RCOSC_LF) {
            PowerCC26XX_module.calLF = true;
        }
    }

    /*
     * if LF source is RCOSC_LF or XOSC_LF: assert the SB_DISALLOW constraint
     * and start a timeout to check for activation
     */
    if ((ccfgLfClkSrc == CCFGREAD_SCLK_LF_OPTION_RCOSC_LF) ||
        (ccfgLfClkSrc == CCFGREAD_SCLK_LF_OPTION_XOSC_LF)) {

        /* disallow STANDBY pending LF clock quailifier disabling */
        Power_setConstraint(PowerCC26XX_SB_DISALLOW);

        /* determine timeout */
        if (ccfgLfClkSrc == CCFGREAD_SCLK_LF_OPTION_RCOSC_LF) {
            timeout = PowerCC26XX_INITIALWAITRCOSC_LF;
        }
        else {
            timeout = PowerCC26XX_INITIALWAITXOSC_LF;
        }

        /* start the Clock object */
        Clock_setTimeout(Clock_handle(&PowerCC26XX_module.lfClockObj),
            (timeout / Clock_tickPeriod));
        Clock_start(Clock_handle(&PowerCC26XX_module.lfClockObj));
    }

    /*
     * else, if the LF clock source is external, can disable clock qualifiers
     * now; no need to assert SB_DISALLOW or start the Clock object
     */
    else if (ccfgLfClkSrc == CCFGREAD_SCLK_LF_OPTION_EXTERNAL_LF) {

        /* yes, disable the LF clock qualifiers */
        DDI16BitfieldWrite(
            AUX_DDI0_OSC_BASE,
            DDI_0_OSC_O_CTL0,
            DDI_0_OSC_CTL0_BYPASS_XOSC_LF_CLK_QUAL_M|
                DDI_0_OSC_CTL0_BYPASS_RCOSC_LF_CLK_QUAL_M,
            DDI_0_OSC_CTL0_BYPASS_RCOSC_LF_CLK_QUAL_S,
            0x3);

        /* enable clock loss detection */
        OSCClockLossEventEnable();
    }

    /* if VIMS RAM is configured as GPRAM: set retention constraint */
    if (!CCFGRead_DIS_GPRAM()) {
        Power_setConstraint(PowerCC26XX_SB_VIMS_CACHE_RETAIN);
    }

    return (Power_SOK);
}

/*
 *  ======== Power_registerNotify ========
 *  Register a function to be called on a specific power event.
 *
 */
int_fast16_t Power_registerNotify(Power_NotifyObj * pNotifyObj,
    uint_fast16_t eventTypes, Power_NotifyFxn notifyFxn, uintptr_t clientArg)
{
    int_fast16_t status = Power_SOK;

    /* check for NULL pointers  */
    if ((pNotifyObj == NULL) || (notifyFxn == NULL)) {
        status = Power_EINVALIDPOINTER;
    }

    else {
        /* fill in notify object elements */
        pNotifyObj->eventTypes = eventTypes;
        pNotifyObj->notifyFxn = notifyFxn;
        pNotifyObj->clientArg = clientArg;

        /* place notify object on event notification queue */
        List_put(&PowerCC26XX_module.notifyList, (List_Elem*)pNotifyObj);
    }

    return (status);
}

/*
 *  ======== Power_releaseConstraint ========
 *  Release a previously declared constraint.
 */
int_fast16_t Power_releaseConstraint(uint_fast16_t constraintId)
{
    unsigned int key;
    uint8_t count;

    /* assert constraintId is valid */
    Assert_isTrue(constraintId < PowerCC26XX_NUMCONSTRAINTS, NULL);

    key = Hwi_disable();

    /* get the count of the constraint */
    count = PowerCC26XX_module.constraintCounts[constraintId];

    /* Assert constraint count is not 0 */
    Assert_isTrue(count != 0, NULL);

    count--;

    /* save the updated count */
    PowerCC26XX_module.constraintCounts[constraintId] = count;

    if (count == 0) {
        PowerCC26XX_module.constraintMask &= ~(1 << constraintId);
    }

    Hwi_restore(key);

    return (Power_SOK);
}

/*
 *  ======== Power_releaseDependency ========
 *  Release a previously declared dependency.
 */
int_fast16_t Power_releaseDependency(uint_fast16_t resourceId)
{
    uint8_t parent;
    uint8_t count;
    uint32_t id;
    unsigned int key;

    /* assert resourceId is valid */
    Assert_isTrue(resourceId < PowerCC26XX_NUMRESOURCES, NULL);

    /* disable interrupts */
    key = Hwi_disable();

    /* read and decrement the reference count */
    count = PowerCC26XX_module.resourceCounts[resourceId];

    /* Assert dependency count is not 0 */
    Assert_isTrue(count != 0, NULL);

    count--;

    /* save the reference count */
    PowerCC26XX_module.resourceCounts[resourceId] = count;

    /* if this was the last dependency being released.., */
    if (count == 0) {
        /* deactivate this resource ... */
        id = resourceDB[resourceId].driverlibID;

        /* is resource a peripheral?... */
        if (resourceDB[resourceId].flags & PowerCC26XX_PERIPH) {
            PRCMPeripheralRunDisable(id);
            PRCMPeripheralSleepDisable(id);
            PRCMPeripheralDeepSleepDisable(id);
            PRCMLoadSet();
            while (!PRCMLoadGet()) {
                ;
            }
        }
        /* else, does resource require a special handler?... */
        else if (resourceDB[resourceId].flags & PowerCC26XX_SPECIAL) {
            /* call the special handler */
            PowerCC26XX_module.resourceHandlers[id](PowerCC26XX_DISABLE);
        }

        /* else resource is a power domain */
        else {
            PRCMPowerDomainOff(id);
            while (PRCMPowerDomainStatus(id) != PRCM_DOMAIN_POWER_OFF) {
                ;
            }
        }

        /* propagate release up the dependency tree ... */

        /* check for a first parent */
        parent = resourceDB[resourceId].flags & PowerCC26XX_PARENTMASK;

        /* if 1st parent, make recursive call to release that dependency */
        if (parent != PowerCC26XX_NOPARENT) {
            Power_releaseDependency(parent);
        }
    }

    /* re-enable interrupts */
    Hwi_restore(key);

    return (Power_SOK);
}

/*
 *  ======== Power_setConstraint ========
 *  Declare an operational constraint.
 */
int_fast16_t Power_setConstraint(uint_fast16_t constraintId)
{
    unsigned int key;

    /* assert constraint id is valid */
    Assert_isTrue(constraintId < PowerCC26XX_NUMCONSTRAINTS, NULL);

    /* disable interrupts */
    key = Hwi_disable();

    /* set the specified constraint in the constraintMask */
    PowerCC26XX_module.constraintMask |= 1 << constraintId;

    /* increment the specified constraint count */
    PowerCC26XX_module.constraintCounts[constraintId]++;

   /* re-enable interrupts */
    Hwi_restore(key);

    return (Power_SOK);
}

/*
 *  ======== Power_setDependency ========
 *  Declare a dependency upon a resource.
 */
int_fast16_t Power_setDependency(uint_fast16_t resourceId)
{
    uint8_t parent;
    uint8_t count;
    uint32_t id;
    unsigned int key;

    /* assert resourceId is valid */
    Assert_isTrue(resourceId < PowerCC26XX_NUMRESOURCES, NULL);

    /* disable interrupts */
    key = Hwi_disable();

    /* read and increment reference count */
    count = PowerCC26XX_module.resourceCounts[resourceId]++;

    /* if resource was NOT activated previously ... */
    if (count == 0) {
        /* propagate set up the dependency tree ... */

        /* check for a first parent */
        parent = resourceDB[resourceId].flags & PowerCC26XX_PARENTMASK;

        /* if first parent, make recursive call to set that dependency */
        if (parent != PowerCC26XX_NOPARENT) {
            Power_setDependency(parent);
        }

        /* now activate this resource ... */
        id = resourceDB[resourceId].driverlibID;

        /* is resource a peripheral?... */
        if (resourceDB[resourceId].flags & PowerCC26XX_PERIPH) {
            PRCMPeripheralRunEnable(id);
            PRCMPeripheralSleepEnable(id);
            PRCMPeripheralDeepSleepEnable(id);
            PRCMLoadSet();
            while (!PRCMLoadGet()) {
                ;
            }
        }
        /* else, does resource require a special handler?... */
        else if (resourceDB[resourceId].flags & PowerCC26XX_SPECIAL) {
            /* call the special handler */
            PowerCC26XX_module.resourceHandlers[id](PowerCC26XX_ENABLE);
        }
        /* else resource is a power domain */
        else {
            PRCMPowerDomainOn(id);
            while (PRCMPowerDomainStatus(id) != PRCM_DOMAIN_POWER_ON) {
                ;
            }
        }
    }

    /* re-enable interrupts */
    Hwi_restore(key);

    return (Power_SOK);
}

/*
 *  ======== Power_setPolicy ========
 *  Set the Power policy function
 */
void Power_setPolicy(Power_PolicyFxn policy)
{
    PowerCC26XX_module.policyFxn = policy;
}

/*
 *  ======== Power_shutdown ========
 */
int_fast16_t Power_shutdown(uint_fast16_t shutdownState,
    uint_fast32_t shutdownTime)
{
    int_fast16_t status = Power_EFAIL;
    unsigned int constraints;
    unsigned int hwiKey;

    /* disable interrupts */
    hwiKey = Hwi_disable();

    /* check if there is a constraint to prohibit shutdown */
    constraints = Power_getConstraintMask();
    if (constraints & (1 << PowerCC26XX_SD_DISALLOW)) {
        status = Power_ECHANGE_NOT_ALLOWED;
    }

    /* OK to shutdown ... */
    else if (PowerCC26XX_module.state == Power_ACTIVE) {
        /* set new transition state to entering shutdown */
        PowerCC26XX_module.state = Power_ENTERING_SHUTDOWN;

        /* signal all clients registered for pre-shutdown notification */
        status = notify(PowerCC26XX_ENTERING_SHUTDOWN);

        /* check for any error */
        if (status != Power_SOK) {
            PowerCC26XX_module.state = Power_ACTIVE;
            Hwi_restore(hwiKey);
            return (status);
        }

        /* now proceed with shutdown sequence ... */

        /* 1. Switch HF, MF, and LF clocks to source from RCOSC_HF */
        if (OSCClockSourceGet(OSC_SRC_CLK_HF) != OSC_RCOSC_HF) {
            OSCClockSourceSet(OSC_SRC_CLK_HF | OSC_SRC_CLK_MF,
                OSC_RCOSC_HF);
            while (!OSCHfSourceReady());
            OSCHfSourceSwitch();
        }

        /* 2. Make sure DMA and CRYTO clocks are off in deep-sleep */
        PRCMPeripheralDeepSleepDisable(PRCM_PERIPH_CRYPTO);
        PRCMPeripheralDeepSleepDisable(PRCM_PERIPH_UDMA);
        PRCMLoadSet();
        while (!PRCMLoadGet()) {
            ;
        }

        /* 3. Power OFF AUX and disconnect from bus */
        AUXWUCPowerCtrl(AUX_WUC_POWER_OFF);

        /* 4. Remove AUX force ON */
        HWREG(AON_WUC_BASE + AON_WUC_O_AUXCTL) &=
            ~AON_WUC_AUXCTL_AUX_FORCE_ON;

        /*
         * 5. Reset AON event source IDs to avoid pending events powering
         * on MCU/AUX
         */
        HWREG(AON_EVENT_BASE + AON_EVENT_O_MCUWUSEL) = 0x3F3F3F3F;
        HWREG(AON_EVENT_BASE + AON_EVENT_O_AUXWUSEL) = 0x003F3F3F;

        /* sync AON */
        SysCtrlAonSync();

        /*
         * 6. Enable shutdown - this latches the IOs, so configuration of
         * IOCFGx registers must be done prior to this
         */
        AONWUCShutDownEnable();

        /* 7. Sync AON */
        SysCtrlAonSync();

        /* 8. Wait until AUX powered off */
        while (AONWUCPowerStatusGet() & AONWUC_AUX_POWER_ON);

        /* 9. Request to power off MCU when go to deep sleep */
        PRCMMcuPowerOff();

        /*
         * 10. Turn off power domains inside MCU VD (BUS, FL_BUS, RFC,
         * CPU)
         */
        PRCMPowerDomainOff(PRCM_DOMAIN_RFCORE | PRCM_DOMAIN_SERIAL |
            PRCM_DOMAIN_PERIPH | PRCM_DOMAIN_CPU | PRCM_DOMAIN_VIMS);

        /* 11. Deep sleep to activate shutdown */
        PRCMDeepSleep();
    }
    else {
        status = Power_EBUSY;
    }

    /* NOTE: if shutdown succeeded, should never get here */

    /* return failure status */
    PowerCC26XX_module.state = Power_ACTIVE;

    /* re-enable interrupts */
    Hwi_restore(hwiKey);

    /* if get here, failed to shutdown, return error code */
    return (status);
}

/*
 *  ======== Power_sleep ========
 */
int_fast16_t Power_sleep(uint_fast16_t sleepState)
{
    int_fast16_t status = Power_SOK;
    int_fast16_t notifyStatus = Power_SOK;
    int_fast16_t lateNotifyStatus = Power_SOK;
    unsigned int xosc_hf_active = FALSE;
    uint_fast16_t postEventLate;
    uint32_t poweredDomains = 0;
    uint_fast16_t preEvent;
    uint_fast16_t postEvent;
    unsigned int constraints;
    bool retainCache = FALSE;
    uint32_t modeVIMS;
    unsigned int taskKey;
    unsigned int swiKey;

    /* first validate the sleep code */
    if (sleepState != PowerCC26XX_STANDBY) {
        status = Power_EINVALIDINPUT;
    }

    else {

        /* check to make sure Power is not busy with another transition */
        if (PowerCC26XX_module.state == Power_ACTIVE) {
            /* set transition state to entering sleep */
            PowerCC26XX_module.state = Power_ENTERING_SLEEP;
        }
        else {
            status = Power_EBUSY;
        }

        if (status == Power_SOK) {

            /* setup sleep vars */
            preEvent = PowerCC26XX_ENTERING_STANDBY;
            postEvent = PowerCC26XX_AWAKE_STANDBY;
            postEventLate = PowerCC26XX_AWAKE_STANDBY_LATE;

            /* disable Task scheduling; allow Swis and Hwis for notifications */
            taskKey = Task_disable();

            /* signal all clients registered for pre-sleep notification */
            status = notify(preEvent);

            /* check for any error */
            if (status != Power_SOK) {
                PowerCC26XX_module.state = Power_ACTIVE;
                Task_restore(taskKey);          /* re-enable scheduler */
                return (status);
            }

            /* now disable Swi scheduling */
            swiKey = Swi_disable();

            /* 1. Freeze the IOs on the boundary between MCU and AON */
            AONIOCFreezeEnable();

            /* 2. If XOSC_HF is active, force it off */
            if(OSCClockSourceGet(OSC_SRC_CLK_HF) == OSC_XOSC_HF) {
                xosc_hf_active = TRUE;
                configureXOSCHF(PowerCC26XX_DISABLE);
            }

            /* 3. Allow AUX to power down */
            AONWUCAuxWakeupEvent(AONWUC_AUX_ALLOW_SLEEP);

            /* 4. Make sure writes take effect */
            SysCtrlAonSync();

            /* now proceed to transition to Power_STANDBY ... */

            /* 5. Query and save domain states before powering them off */
            if (Power_getDependencyCount(PowerCC26XX_DOMAIN_RFCORE)) {
                poweredDomains |= PRCM_DOMAIN_RFCORE;
            }
            if (Power_getDependencyCount(PowerCC26XX_DOMAIN_SERIAL)){
                poweredDomains |= PRCM_DOMAIN_SERIAL;
            }
            if (Power_getDependencyCount(PowerCC26XX_DOMAIN_PERIPH)) {
                poweredDomains |= PRCM_DOMAIN_PERIPH;
            }

            /* 6. Gate running deep sleep clocks for Crypto and DMA */
            if (Power_getDependencyCount(PowerCC26XX_PERIPH_CRYPTO)) {
                PRCMPeripheralDeepSleepDisable(
                    resourceDB[PowerCC26XX_PERIPH_CRYPTO].driverlibID);
            }
            if (Power_getDependencyCount(PowerCC26XX_PERIPH_UDMA)) {
                PRCMPeripheralDeepSleepDisable(
                    resourceDB[PowerCC26XX_PERIPH_UDMA].driverlibID);
            }
            /* 7. Make sure clock settings take effect */
            PRCMLoadSet();

            /* 8. Request power off of domains in the MCU voltage domain */
            PRCMPowerDomainOff(poweredDomains | PRCM_DOMAIN_CPU);

            /* 9. Request uLDO during standby */
            PRCMMcuUldoConfigure(true);

            /* query constraints to determine if cache should be retained */
            constraints = Power_getConstraintMask();
            if (constraints & (1 << PowerCC26XX_SB_VIMS_CACHE_RETAIN)) {
                retainCache = TRUE;
            }

            /* 10. If don't want VIMS retention in standby, disable it now... */
            if (retainCache == FALSE) {

                /* 10.1 Get the current VIMS mode */
                do {
                    modeVIMS = VIMSModeGet(VIMS_BASE);
                } while (modeVIMS == VIMS_MODE_CHANGING);

                /* 10.2 If in a cache mode, turn VIMS off */
                if (modeVIMS == VIMS_MODE_ENABLED) {

                    /* 10.3 Now turn off the VIMS */
                    VIMSModeSet(VIMS_BASE, VIMS_MODE_OFF);
                }

                /* 10.4 Now disable retention */
                PRCMCacheRetentionDisable();
            }

            /* 11. Setup recharge parameters */
            SysCtrlSetRechargeBeforePowerDown(XOSC_IN_HIGH_POWER_MODE);

            /* 12. Make sure all writes have taken effect */
            SysCtrlAonSync();

            /* 13. Invoke deep sleep to go to STANDBY */
            PRCMDeepSleep();

            /* 14. If didn't retain VIMS in standby, re-enable retention now */
            if (retainCache == FALSE) {

                /* 14.1 If previously in a cache mode, restore the mode now */
                if (modeVIMS == VIMS_MODE_ENABLED) {
                    VIMSModeSet(VIMS_BASE, modeVIMS);
                }

                /* 14.2 Re-enable retention */
                PRCMCacheRetentionEnable();
            }

            /* 15. Start forcing on power to AUX */
            AONWUCAuxWakeupEvent(AONWUC_AUX_WAKEUP);

            /* 16. Start re-powering power domains */
            PRCMPowerDomainOn(poweredDomains);

            /* 17. Restore deep sleep clocks of Crypto and DMA */
            if (Power_getDependencyCount(PowerCC26XX_PERIPH_CRYPTO)) {
                PRCMPeripheralDeepSleepEnable(
                    resourceDB[PowerCC26XX_PERIPH_CRYPTO].driverlibID);
            }
            if (Power_getDependencyCount(PowerCC26XX_PERIPH_UDMA)) {
                PRCMPeripheralDeepSleepEnable(
                    resourceDB[PowerCC26XX_PERIPH_UDMA].driverlibID);
            }

            /* 18. Make sure clock settings take effect */
            PRCMLoadSet();

            /* 19. Release request for uLDO */
            PRCMMcuUldoConfigure(false);

            /* 20. Set transition state to EXITING_SLEEP */
            PowerCC26XX_module.state = Power_EXITING_SLEEP;

            /* 21. Wait until all power domains are back on */
            while (PRCMPowerDomainStatus(poweredDomains) !=
                   PRCM_DOMAIN_POWER_ON) {
                ;
            }

            /* 22. Wait for the RTC shadow values to be updated so that
             * the early notification callbacks can read out valid RTC values
             */
            SysCtrlAonSync();

            /*
             * 23. Signal clients registered for early post-sleep notification;
             * this should be used to initialize any timing critical or IO
             * dependent hardware
             */
            notifyStatus = notify(postEvent);

            /* 24. Disable IO freeze and ensure RTC shadow value is updated */
            AONIOCFreezeDisable();
            SysCtrlAonSync();

            /* 25. Wait for AUX to power up */
            while(!(AONWUCPowerStatusGet() & AONWUC_AUX_POWER_ON)) {};

            /* 26. If XOSC_HF was forced off above, initiate switch back */
            if (xosc_hf_active == TRUE) {
                configureXOSCHF(PowerCC26XX_ENABLE);
            }

            /* 27. Re-enable interrupts */
            CPUcpsie();

            /*
             * 28. Signal all clients registered for late post-sleep
             * notification
             */
            lateNotifyStatus = notify(postEventLate);

            /*
             * 29. Now clear the transition state before re-enabling
             * scheduler
             */
            PowerCC26XX_module.state = Power_ACTIVE;

            /* 30. Re-enable Swi scheduling */
            Swi_restore(swiKey);

            /* 31. Adjust recharge parameters */
            SysCtrlAdjustRechargeAfterPowerDown();

            /* re-enable Task scheduling */
            Task_restore(taskKey);

            /* if there was a notification error, set return status */
            if ((notifyStatus != Power_SOK) ||
                (lateNotifyStatus != Power_SOK)) {
                status = Power_EFAIL;
            }
        }
    }

    return (status);
}

/*
 *  ======== Power_unregisterNotify ========
 *  Unregister for a power notification.
 *
 */
void Power_unregisterNotify(Power_NotifyObj * pNotifyObj)
{
    unsigned int key;

    /* remove notify object from its event queue */
    key = Hwi_disable();

    /* remove notify object from its event queue */
    List_remove(&PowerCC26XX_module.notifyList, (List_Elem *)pNotifyObj);

    Hwi_restore(key);
}

/* ****************** CC26XX specific APIs ******************** */

/*
 *  ======== PowerCC26XX_calibrate ========
 *  Plug this function into the PowerCC26XX_Config structure
 *  if calibration is needed.
 */
bool PowerCC26XX_calibrate(unsigned int arg)
{
    bool retVal = FALSE;
    Clock_Params clockParams;

    switch (arg) {
        case PowerCC26XX_SETUP_CALIBRATE:
            /*
             *  If RCOSC calibration is enabled, construct a Clock object for
             *  delays. Set timeout to '1' Clock tick period for the minimal
             *  delay. The object will explicitly started by Power module when
             *  appropriate
             */
            Clock_Params_init(&clockParams);
            clockParams.period = 0;
            clockParams.startFlag = FALSE;
            clockParams.arg = 0;
            Clock_construct(&PowerCC26XX_module.calClockStruct,
                &PowerCC26XX_RCOSC_clockFunc, 1, &clockParams);

            /* construct the Hwi */
            Hwi_construct(&PowerCC26XX_module.hwiStruct,
                44, PowerCC26XX_auxISR, NULL, NULL);

            break;

        case PowerCC26XX_INITIATE_CALIBRATE:
            retVal = PowerCC26XX_initiateCalibration();
            break;

        case PowerCC26XX_DO_CALIBRATE:
            PowerCC26XX_doCalibrate();
            break;
    }

    return (retVal);
}

/*
 *  ======== PowerCC26XX_doWFI ========
 */
void PowerCC26XX_doWFI(void)
{
    __asm(" wfi");
}

/*
 *  ======== PowerCC26XX_getClockHandle ========
 */
Clock_Handle PowerCC26XX_getClockHandle()
{
    return ((Clock_Handle)&PowerCC26XX_module.clockObj);
}

/*
 *  ======== PowerCC26XX_noCalibrate ========
 *  Plug this function into the PowerCC26XX config structure if calibration
 *  is not needed.
 */
bool PowerCC26XX_noCalibrate(unsigned int arg)
{
    return (0);
}

/*
 *  ======== PowerCC26XX_getXoscStartupTime ========
 *  Get the estimated crystal oscillator startup time
 */
uint32_t PowerCC26XX_getXoscStartupTime(uint32_t timeUntilWakeupInMs)
{
    return (OSCHF_GetStartupTime(timeUntilWakeupInMs));
}

/*
 *  ======== PowerCC26XX_injectCalibration ========
 *  Explicitly trigger RCOSC calibration
 */
bool PowerCC26XX_injectCalibration(void)
{
    if ((*(PowerCC26XX_config.calibrateFxn))(PowerCC26XX_INITIATE_CALIBRATE)) {
        /* here if AUX SMPH was available, start calibration now ... */
        (*(PowerCC26XX_config.calibrateFxn))(PowerCC26XX_DO_CALIBRATE);
        return (TRUE);
    }

    return (FALSE);
}

/*
 *  ======== PowerCC26XX_isStableXOSC_HF ========
 *  Check if XOSC_HF has stabilized.
 */
bool PowerCC26XX_isStableXOSC_HF(void)
{
    bool ready = TRUE;
    unsigned int key;

    key = Hwi_disable();

    /* only query if HF source is ready if there is a pending change */
    if (PowerCC26XX_module.xoscPending) {
        ready = OSCHfSourceReady();
    }

    Hwi_restore(key);

    return (ready);
}

/*
 *  ======== PowerCC26XX_switchXOSC_HF ========
 *  Switch to enable XOSC_HF.
 *  May only be called when using the PowerCC26XX_DISALLOW_XOSC_HF_SWITCHING
 *  constraint.
 *  May only be called after ensuring the XOSC_HF is stable by calling
 *  PowerCC26XX_isStableXOSC_HF().
 */
void PowerCC26XX_switchXOSC_HF(void)
{
    bool readyToCal;
    unsigned int key;

    key = Hwi_disable();

    /* Since PowerCC26XX_isStableXOSC_HF() should have been called before this
     * function, we can just switch without handling the case when the XOSC_HF
     * is not ready or PowerCC26XX_module.xoscPending is not TRUE.
     */
    OSCHF_AttemptToSwitchToXosc();

    /* Since configureXOSCHF() was called prior to this function to turn
     * on the XOSC_HF, PowerCC26XX_module.xoscPending will be TRUE and
     * we can safely set it to FALSE.
     */
    PowerCC26XX_module.xoscPending = FALSE;


    /* initiate RCOSC calibration */
    readyToCal = (*(PowerCC26XX_config.calibrateFxn))(PowerCC26XX_INITIATE_CALIBRATE);

    /* now notify clients that were waiting for a switch notification */
    notify(PowerCC26XX_XOSC_HF_SWITCHED);

    /* if ready to start first cal measurment, do it now */
    if (readyToCal == TRUE) {
        (*(PowerCC26XX_config.calibrateFxn))(PowerCC26XX_DO_CALIBRATE);
    }

    Hwi_restore(key);
}

/* * * * * * * * * * * internal and support functions * * * * * * * * * * */

/*
 *  ======== emptyClockFunc ========
 *  Clock function used by power policy to schedule early wakeups.
 */
static void emptyClockFunc(uintptr_t arg)
{
}

/*
 *  ======== disableLFClockQualifiers ========
 *  Clock function used for delayed disable of LF clock qualifiers.
 */
static void disableLFClockQualifiers(uintptr_t arg)
{
    uint32_t ccfgLfClkSrc;
    uint32_t sourceLF;
    uint32_t timeout;

     /* query LF clock source */
    sourceLF = OSCClockSourceGet(OSC_SRC_CLK_LF);

    /* is LF source either RCOSC_LF or XOSC_LF yet? */
    if ((sourceLF == OSC_RCOSC_LF) || (sourceLF == OSC_XOSC_LF)) {

        /* yes, disable the LF clock qualifiers */
        DDI16BitfieldWrite(
            AUX_DDI0_OSC_BASE,
            DDI_0_OSC_O_CTL0,
            DDI_0_OSC_CTL0_BYPASS_XOSC_LF_CLK_QUAL_M|
                DDI_0_OSC_CTL0_BYPASS_RCOSC_LF_CLK_QUAL_M,
            DDI_0_OSC_CTL0_BYPASS_RCOSC_LF_CLK_QUAL_S,
            0x3
        );

        /* enable clock loss detection */
        OSCClockLossEventEnable();

        /* now finish by releasing the standby disallow constraint */
        Power_releaseConstraint(PowerCC26XX_SB_DISALLOW);
    }

    /* not yet, LF still derived from HF, restart clock to check back later */
    else {
        /* read the LF clock source from CCFG */
        ccfgLfClkSrc = CCFGRead_SCLK_LF_OPTION();

        /* determine retry timeout */
        if (ccfgLfClkSrc == CCFGREAD_SCLK_LF_OPTION_RCOSC_LF) {
            timeout = PowerCC26XX_RETRYWAITRCOSC_LF;
        }
        else {
            timeout = PowerCC26XX_RETRYWAITXOSC_LF;
        }
        /* retrigger LF Clock to fire again */
        Clock_setTimeout(Clock_handle(&PowerCC26XX_module.lfClockObj),
            (timeout / Clock_tickPeriod));
        Clock_start(Clock_handle(&PowerCC26XX_module.lfClockObj));
    }
}

/*
 *  ======== nopResourceFunc ========
 *  special resource handler
 */
static unsigned int nopResourceHandler(unsigned int action)
{
    return (0);
}

/*
 *  ======== notify ========
 *  Send notifications to registered clients.
 *  Note: Task scheduling is disabled when this function is called.
 */
static int_fast16_t notify(uint_fast16_t eventType)
{
    int_fast16_t notifyStatus;
    Power_NotifyFxn notifyFxn;
    uintptr_t clientArg;
    List_Elem *elem;

    /* if queue is empty, return immediately */
    if (!List_empty(&PowerCC26XX_module.notifyList)) {
        /* point to first client notify object */
        elem = List_head(&PowerCC26XX_module.notifyList);

        /* walk the queue and notify each registered client of the event */
        do {
            if (((Power_NotifyObj *)elem)->eventTypes & eventType) {
                /* pull params from notify object */
                notifyFxn = ((Power_NotifyObj *)elem)->notifyFxn;
                clientArg = ((Power_NotifyObj *)elem)->clientArg;

                /* call the client's notification function */
                notifyStatus = (int_fast16_t)(*(Power_NotifyFxn)notifyFxn)(
                    eventType, 0, clientArg);

                /* if client declared error stop all further notifications */
                if (notifyStatus != Power_NOTIFYDONE) {
                    return (Power_EFAIL);
                }
            }

            /* get next element in the notification queue */
            elem = List_next(elem);

        } while (elem != NULL);
    }

    return (Power_SOK);
}

/*
 *  ======== configureRFCoreClocks ========
 *  Special dependency function for controlling RF core clocks.
 */
static unsigned int configureRFCoreClocks(unsigned int action)
{
    if (action == PowerCC26XX_ENABLE) {
        RFCClockEnable();
    }
    else {
        RFCClockDisable();
    }

    return (0);
}

/*
 *  ======== switchXOSCHFclockFunc ========
 *  Clock function used for delayed switching to XOSC_HF.
 */
static void switchXOSCHFclockFunc(uintptr_t arg0)
{
    bool readyToCal;
    uint32_t timeout;
    unsigned int key;

    key = Hwi_disable();

    /* if pending switch has already been made, just send out notifications */
    if (PowerCC26XX_module.xoscPending == FALSE) {

        /* initiate RCOSC calibration */
        readyToCal = (*(PowerCC26XX_config.calibrateFxn))(PowerCC26XX_INITIATE_CALIBRATE);

        /* notify clients that were waiting for a switch notification */
        notify(PowerCC26XX_XOSC_HF_SWITCHED);

        /* if ready to start first cal measurment, do it now */
        if (readyToCal == TRUE) {
            (*(PowerCC26XX_config.calibrateFxn))(PowerCC26XX_DO_CALIBRATE);
        }
    }

    /* else, if HF ready to switch, do it now ... */
    else if (OSCHF_AttemptToSwitchToXosc()) {

        PowerCC26XX_module.xoscPending = FALSE;

        /* initiate RCOSC calibration */
        readyToCal = (*(PowerCC26XX_config.calibrateFxn))(PowerCC26XX_INITIATE_CALIBRATE);

        /* now notify clients that were waiting for a switch notification */
        notify(PowerCC26XX_XOSC_HF_SWITCHED);

        /* if ready to start first cal measurment, do it now */
        if (readyToCal == TRUE) {
            (*(PowerCC26XX_config.calibrateFxn))(PowerCC26XX_DO_CALIBRATE);
        }
    }

    /* else, wait some more, then see if can switch ... */
    else {
        /* calculate wait timeout in units of ticks */
        timeout = PowerCC26XX_RETRYWAITXOSC_HF / Clock_tickPeriod;
        if (timeout == 0) {
            timeout = 1;   /* wait at least 1 tick */
        }

        /* re-start Clock object with retry timeout */
        Clock_setTimeout(
            Clock_handle(&PowerCC26XX_module.xoscClockObj), timeout);
        Clock_start(Clock_handle(&PowerCC26XX_module.xoscClockObj));
    }

    Hwi_restore(key);
}

/*
 *  ======== configureXOSCHF ========
 */
static unsigned int configureXOSCHF(unsigned int action)
{
    uint32_t timeout;

    if (action == PowerCC26XX_ENABLE && OSCClockSourceGet(OSC_SRC_CLK_HF) != OSC_XOSC_HF) {
        OSCHF_TurnOnXosc();

        PowerCC26XX_module.xoscPending = TRUE;

        /* Unless it is disallowed, estimate the required stabilisation
         * time and start a clock.
         * When the clock times out, the callback will try and switch to
         * the XOSC_HF. If the XOSC_HF is not ready yet, the callback
         * will start a new clock to try again.
         */
        if (!(Power_getConstraintMask() & (1 << PowerCC26XX_DISALLOW_XOSC_HF_SWITCHING))) {
             /* calculate wait timeout in units of ticks */
            timeout = PowerCC26XX_INITIALWAITXOSC_HF / Clock_tickPeriod;
            if (timeout == 0) {
                timeout = 1;   /* wait at least 1 tick */
            }

            /* start Clock object with initial timeout */
            Clock_stop(Clock_handle(&PowerCC26XX_module.xoscClockObj));
            Clock_setTimeout(Clock_handle(&PowerCC26XX_module.xoscClockObj),
                timeout);
            Clock_start(Clock_handle(&PowerCC26XX_module.xoscClockObj));
        }
    }

    /* when release XOSC_HF, auto switch to RCOSC_HF */
    else {
        OSCHF_SwitchToRcOscTurnOffXosc();
    }
    return (0);
}
