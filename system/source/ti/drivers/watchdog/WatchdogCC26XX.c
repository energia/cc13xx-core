/*
 * Copyright (c) 2015-2016, Texas Instruments Incorporated
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

#include <stdint.h>
#include <stdlib.h>

#include <xdc/runtime/Error.h>
#include <xdc/runtime/Assert.h>
#include <xdc/runtime/Diags.h>
#include <xdc/runtime/Log.h>
#include <xdc/runtime/Types.h>

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/family/arm/m3/Hwi.h>
#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26XX.h>

#include <ti/drivers/watchdog/WatchdogCC26XX.h>

#ifdef DEVICE_FAMILY
    #undef DEVICE_FAMILY_PATH
    #define DEVICE_FAMILY_PATH(x) <ti/devices/DEVICE_FAMILY/x>
#else
    #error "You must define DEVICE_FAMILY at the project level as one of cc26x0, cc26x0r2, cc13x0, etc."
#endif

#include DEVICE_FAMILY_PATH(driverlib/watchdog.h)

/* Function prototypes */
void WatchdogCC26XX_clear(Watchdog_Handle handle);
void WatchdogCC26XX_close(Watchdog_Handle handle);
int_fast16_t  WatchdogCC26XX_control(Watchdog_Handle handle, uint_fast16_t cmd,
        void *arg);
void WatchdogCC26XX_init(Watchdog_Handle handle);
Watchdog_Handle WatchdogCC26XX_open(Watchdog_Handle handle, Watchdog_Params *params);
int_fast16_t WatchdogCC26XX_setReload(Watchdog_Handle handle,
        uint32_t ticks);
uint32_t WatchdogCC26XX_convertMsToTicks(uint32_t milliseconds);

/* WatchdogCC26XX internal functions */
static void WatchdogCC26XX_initHw(Watchdog_Handle handle);

/* Watchdog function table for CC26XX implementation */
const Watchdog_FxnTable WatchdogCC26XX_fxnTable = {
    WatchdogCC26XX_clear,
    WatchdogCC26XX_close,
    WatchdogCC26XX_control,
    WatchdogCC26XX_init,
    WatchdogCC26XX_open,
    WatchdogCC26XX_setReload,
    WatchdogCC26XX_convertMsToTicks
};

/* Maximum allowable setReload value */
#define MAX_RELOAD_VALUE        0xFFFFFFFF
#define WATCHDOG_DIV_RATIO      32            /* Watchdog division ratio */
#define MS_RATIO                1000          /* millisecond to second ratio */

/*
 *  ======== WatchdogCC26XX_clear ========
 */
void WatchdogCC26XX_clear(Watchdog_Handle handle)
{
    WatchdogIntClear();
}

/*
 *  ======== WatchdogCC26XX_close ========
 */
void WatchdogCC26XX_close(Watchdog_Handle handle)
{
    /*
     *  Not supported for CC26XX - Once the INTEN bit of the WDTCTL
     *  register has been set, it can only be cleared by a hardware
     *  reset.
     */
    Assert_isTrue(false, NULL);
}

/*
 *  ======== WatchdogCC26XX_control ========
 *  @pre    Function assumes that the handle is not NULL
 */
int_fast16_t WatchdogCC26XX_control(Watchdog_Handle handle, uint_fast16_t cmd,
        void *arg)
{
    /* No implementation yet */
    return (Watchdog_STATUS_UNDEFINEDCMD);
}

/*
 *  ======== Watchdog_init ========
 */
void WatchdogCC26XX_init(Watchdog_Handle handle)
{
    WatchdogCC26XX_Object *object = handle->object;

    object->isOpen = false;
}

/*
 *  ======== WatchdogCC26XX_open ========
 */
Watchdog_Handle WatchdogCC26XX_open(Watchdog_Handle handle, Watchdog_Params *params)
{
    unsigned int                   key;
    Hwi_Params                     hwiParams;
    WatchdogCC26XX_Object         *object;

    /* get the pointer to the object and hwAttrs */
    object = handle->object;

    /* disable preemption while checking if the WatchDog is open. */
    key = Hwi_disable();

    /* Check if the Watchdog is open already with the HWAttrs */
    if (object->isOpen == true) {
        Hwi_restore(key);
        Log_warning1("Watchdog: Handle %x already in use.", (UArg)handle);
        return (NULL);
    }

    object->isOpen = true;
    Hwi_restore(key);

    /* initialize the Watchdog object */
    object->debugStallMode = params->debugStallMode;
    object->resetMode      = params->resetMode;

    /* Construct Hwi object for Watchdog */
    Hwi_Params_init(&hwiParams);
    hwiParams.arg = (UArg)handle;

    /* setup callback function if defined */
    if (params->callbackFxn != NULL) {
        Hwi_plug(INT_NMI_FAULT, (void *)params->callbackFxn);
    }

    /* initialize the watchdog hardware */
    WatchdogCC26XX_initHw(handle);

    Log_print1(Diags_USER1, "Watchdog: handle %x opened" ,(UArg)handle);

    /* return handle of the Watchdog object */
    return (handle);
}

/*
 *  ======== WatchdogCC26XX_setReload ========
 */
int_fast16_t WatchdogCC26XX_setReload(Watchdog_Handle handle, uint32_t ticks)
{
    unsigned int                   key;

    /* disable preemption while unlocking WatchDog registers */
    key = Hwi_disable();

    /* unlock the Watchdog configuration registers */
    WatchdogUnlock();

    /* make sure the Watchdog is unlocked before continuing */
    while(WatchdogLockState() == WATCHDOG_LOCK_LOCKED)
    { }

    /* update the reload value */
    WatchdogReloadSet(ticks);

    /* lock register access */
    WatchdogLock();

    Hwi_restore(key);

    Log_print2(Diags_USER1, "Watchdog: WDT with handle 0x%x has been set to "
        "reload to 0x%x", (UArg)handle, ticks);

    return (Watchdog_STATUS_SUCCESS);
}

/*
 *  ======== WatchdogCC26XX_hwInit ========
 *  This function initializes the Watchdog hardware module.
 *
 *  @pre    Function assumes that the Watchdog handle is pointing to a hardware
 *          module which has already been opened.
 */
static void WatchdogCC26XX_initHw(Watchdog_Handle handle) {
    unsigned int                   key;
    uint32_t                       tickValue;
    WatchdogCC26XX_Object          *object;
    WatchdogCC26XX_HWAttrs const   *hwAttrs;

    /* get the pointer to the object and hwAttrs */
    object = handle->object;
    hwAttrs = handle->hwAttrs;

    /* convert milliseconds to watchdog timer ticks */
    tickValue = WatchdogCC26XX_convertMsToTicks(hwAttrs->reloadValue);

    /* disable preemption while unlocking WatchDog registers */
    key = Hwi_disable();

    /* unlock the Watchdog configuration registers */
    WatchdogUnlock();

    /* make sure the Watchdog is unlocked before continuing */
    while(WatchdogLockState() == WATCHDOG_LOCK_LOCKED)
    { }

    WatchdogReloadSet(tickValue);

    /* set reset mode */
    if (object->resetMode == Watchdog_RESET_ON) {
        WatchdogResetEnable();
    }
    else {
        WatchdogResetDisable();
    }

    /* set debug stall mode */
    if (object->debugStallMode == Watchdog_DEBUG_STALL_ON) {
        WatchdogStallEnable();
    }
    else {
        WatchdogStallDisable();
    }

    /* enable the Watchdog interrupt as a non-maskable interrupt */
    WatchdogIntTypeSet(WATCHDOG_INT_TYPE_NMI);

    /* enable the Watchdog */
    WatchdogEnable();

    /* lock the Watchdog configuration registers */
    WatchdogLock();

    Hwi_restore(key);
}

/*
 *  ======== WatchdogCC26XX_convertMsToTicks ========
 *  This function converts the input value from milliseconds to
 *  Watchdog clock ticks.
 */
uint32_t WatchdogCC26XX_convertMsToTicks(uint32_t milliseconds)
{
    uint32_t                       tickValue;
    uint32_t                       convertRatio;
    uint32_t                       maxConvertMs;
    Types_FreqHz                   freq;

    /* Determine milliseconds to clock ticks conversion ratio */
    /* Watchdog clock ticks/sec = CPU clock / WATCHDOG_DIV_RATIO */
    /* Watchdog clock ticks/ms = CPU clock / WATCHDOG_DIV_RATIO / 1000 */
    BIOS_getCpuFreq(&freq);
    convertRatio = freq.lo / WATCHDOG_DIV_RATIO / MS_RATIO;
    maxConvertMs = MAX_RELOAD_VALUE / convertRatio;

    /* convert milliseconds to watchdog timer ticks */
    /* check if value exceeds maximum */
    if (milliseconds > maxConvertMs)
    {
        tickValue = 0;  /* return zero to indicate overflow */
    }
    else
    {
        tickValue = (uint32_t)(milliseconds * convertRatio);
    }

    return(tickValue);
}
