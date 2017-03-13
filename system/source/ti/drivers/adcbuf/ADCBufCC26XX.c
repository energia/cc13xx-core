/*
 * Copyright (c) 2016, Texas Instruments Incorporated
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
#include <xdc/runtime/Error.h>
#include <xdc/runtime/Assert.h>
#include <xdc/runtime/Diags.h>
#include <xdc/runtime/Log.h>
#include <xdc/runtime/Types.h>

/* Kernel services */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/family/arm/m3/Hwi.h>
#include <ti/sysbios/knl/Swi.h>

/* TI-RTOS drivers */
#include <ti/drivers/ADCBuf.h>
#include <ti/drivers/adcbuf/ADCBufCC26XX.h>
#include <ti/drivers/PIN.h>
#include <ti/drivers/pin/PINCC26XX.h>
#include <ti/drivers/dma/UDMACC26XX.h>
#include <ti/drivers/timer/GPTimerCC26XX.h>
#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26XX.h>

/* driverlib header files */
#ifdef DEVICE_FAMILY
    #undef DEVICE_FAMILY_PATH
    #define DEVICE_FAMILY_PATH(x) <ti/devices/DEVICE_FAMILY/x>
#else
    #error "You must define DEVICE_FAMILY at the project level as one of cc26x0, cc26x0r2, cc13x0, etc."
#endif

#include DEVICE_FAMILY_PATH(inc/hw_memmap.h)
#include DEVICE_FAMILY_PATH(inc/hw_ints.h)
#include DEVICE_FAMILY_PATH(inc/hw_types.h)
#include DEVICE_FAMILY_PATH(inc/hw_aux_evctl.h)
#include DEVICE_FAMILY_PATH(driverlib/aux_adc.h)
#include DEVICE_FAMILY_PATH(driverlib/aux_wuc.h)
#include DEVICE_FAMILY_PATH(driverlib/aux_smph.h)
#include DEVICE_FAMILY_PATH(driverlib/sys_ctrl.h)
#include DEVICE_FAMILY_PATH(driverlib/ioc.h)
#include DEVICE_FAMILY_PATH(driverlib/aon_ioc.h)

/*
 * =============================================================================
 * Public Function Declarations
 * =============================================================================
 */
void ADCBufCC26XX_init(ADCBuf_Handle handle);
ADCBuf_Handle ADCBufCC26XX_open(ADCBuf_Handle handle, const ADCBuf_Params *params);
int_fast16_t ADCBufCC26XX_convert(ADCBuf_Handle handle, ADCBuf_Conversion conversions[],  uint_fast8_t channelCount);
int_fast16_t ADCBufCC26XX_convertCancel(ADCBuf_Handle handle);
void ADCBufCC26XX_close(ADCBuf_Handle handle);
uint_fast8_t ADCBufCC26XX_getResolution(ADCBuf_Handle handle);
int_fast16_t ADCBufCC26XX_adjustRawValues(ADCBuf_Handle handle, void *sampleBuffer, uint_fast16_t sampleCount, uint32_t adcChannel);
int_fast16_t ADCBufCC26XX_convertAdjustedToMicroVolts(ADCBuf_Handle handle, uint32_t  adcChannel, void *adjustedSampleBuffer, uint32_t outputMicroVoltBuffer[], uint_fast16_t sampleCount);
int_fast16_t ADCBufCC26XX_control(ADCBuf_Handle handle, uint_fast16_t cmd, void *arg);

/*
 * =============================================================================
 * Private Function Declarations
 * =============================================================================
 */
static bool ADCBufCC26XX_acquireADCSemaphore(ADCBuf_Handle handle);
static bool ADCBufCC26XX_releaseADCSemaphore(ADCBuf_Handle handle);
static void ADCBufCC26XX_configDMA(ADCBuf_Handle handle, ADCBuf_Conversion *conversion);
static void ADCBufCC26XX_hwiFxn (UArg arg);
static void ADCBufCC26XX_swiFxn (UArg arg0, UArg arg1);
static void ADCBufCC26XX_conversionCallback(ADCBuf_Handle handle, ADCBuf_Conversion *conversion, void *completedADCBuffer, uint32_t completedChannel);
static uint32_t ADCBufCC26XX_freqToCounts(uint32_t frequency);
static void ADCBufCC26XX_configGPTDMA(ADCBuf_Handle handle, ADCBuf_Conversion *conversion);
static void ADCBufCC26XX_cleanADC(ADCBuf_Handle handle);
static void ADCBufCC26XX_loadDMAControlTableEntry(ADCBuf_Handle handle, ADCBuf_Conversion *conversion, bool primaryEntry);
static void ADCBufCC26XX_loadGPTDMAControlTableEntry(ADCBuf_Handle handle, ADCBuf_Conversion *conversion, bool primaryEntry);

/*
 * =============================================================================
 * Constants
 * =============================================================================
 */

const ADCBuf_FxnTable ADCBufCC26XX_fxnTable = {
    /*! Function to close the specified peripheral */
    ADCBufCC26XX_close,
    /*! Function to driver implementation specific control function */
    ADCBufCC26XX_control,
    /*! Function to initialize the given data object */
    ADCBufCC26XX_init,
    /*! Function to open the specified peripheral */
    ADCBufCC26XX_open,
    /*! Function to start an ADC conversion with the specified peripheral */
    ADCBufCC26XX_convert,
    /*! Function to abort a conversion being carried out by the specified peripheral */
    ADCBufCC26XX_convertCancel,
    /*! Function to get the resolution in bits of the ADC */
    ADCBufCC26XX_getResolution,
    /*! Function to adjust raw ADC output values to values comparable between devices of the same type */
    ADCBufCC26XX_adjustRawValues,
    /*! Function to convert adjusted ADC values to microvolts */
    ADCBufCC26XX_convertAdjustedToMicroVolts
};


/*
 * =============================================================================
 * Private Global Variables
 * =============================================================================
 */

/* Allocate space for DMA control table entry */
ALLOCATE_CONTROL_TABLE_ENTRY(dmaADCPriControlTableEntry, (UDMA_CHAN_AUX_ADC + UDMA_PRI_SELECT));
ALLOCATE_CONTROL_TABLE_ENTRY(dmaADCAltControlTableEntry, (UDMA_CHAN_AUX_ADC + UDMA_ALT_SELECT));
ALLOCATE_CONTROL_TABLE_ENTRY(dmaGPT0APriControlTableEntry, (UDMA_CHAN_TIMER0_A + UDMA_PRI_SELECT));
ALLOCATE_CONTROL_TABLE_ENTRY(dmaGPT0AAltControlTableEntry, (UDMA_CHAN_TIMER0_A + UDMA_ALT_SELECT));

/*!
 *  Timeout interrupt bitmask of the GPT that the DMA copies into GPT_O_ICLR
 *  to bring the DMA trigger event low.
 *  Needs to go into RAM not flash to prevent system from hanging in edge cases.
 */
static uint8_t gptClear = GPT_ICLR_TATOCINT;

/*
 * =============================================================================
 * Function Definitions
 * =============================================================================
 */

/*!
 *  @brief  Function to initializes the ADCBufCC26XX driver
 *
 *  @pre    The ADCBufCC26XX_config structure must exist and be persistent before this
 *          function can be called. This function must also be called before
 *          any other ADC driver APIs.
 */
void ADCBufCC26XX_init(ADCBuf_Handle handle) {
    ADCBufCC26XX_Object        *object;

    /* Get the pointer to the object */
    object = handle->object;
    /* Mark the object as available */
    object->isOpen = false;
}


/*!
 *  @brief  Function to initialize the ADCBufCC26XX peripheral
 *
 *  Function to initialize the ADCBufCC26XX peripheral specified by the
 *  particular index value.
 *
 *  @pre    ADCBufCC26XX_init() has been called
 *
 *  @param  handle        An uninitialised ADCBufCC26XX_Handle
 *
 *  @param  params        Pointer to a parameter block, if NULL it will use
 *                        default values. All the fields in this structure are
 *                        RO (read-only).
 *
 *  @return An ADCBufCC26XX_Handle on success or a NULL on an error or if it has been
 *          opened already.
 *
 *  @sa     ADCBufCC26XX_init()
 *  @sa     ADCBufCC26XX_close()
 */
ADCBuf_Handle ADCBufCC26XX_open(ADCBuf_Handle handle,
                                const ADCBuf_Params *params) {
    /* Use union to save on stack allocation */
    union {
        Semaphore_Params        semParams;
        Hwi_Params              hwiParams;
        Swi_Params              swiParams;
        GPTimerCC26XX_Params    timerParams;
    } paramsUnion;
    ADCBufCC26XX_Object         *object;
    ADCBufCC26XX_HWAttrs  const *hwAttrs;
    uint32_t                    key;
    uint32_t                    adcPeriodCounts;

    /* Get the pointer to the object and hwAttrs */
    object = handle->object;
    hwAttrs = handle->hwAttrs;

    /* Disable preemption while checking if the ADC is open. */
    key = Hwi_disable();

    /* Check if the ADC is open already with the base addr. */
    if (object->isOpen == true) {
        Hwi_restore(key);

        Log_warning0("ADCBuf: already in use.");

        return (NULL);
    }

    /* Mark the handle as being used */
    object->isOpen = true;

    /* Turn on the ANAIF clock. ANIAF contains the aux ADC.
     */
    AUXWUCClockEnable(AUX_WUC_ANAIF_CLOCK);
    AUXWUCClockEnable(AUX_WUC_ADI_CLOCK);

    Hwi_restore(key);

    if (params->returnMode == ADCBuf_RETURN_MODE_BLOCKING) {
        /* Continuous trigger mode and blocking return mode is an illegal combination */
        Assert_isTrue(!(params->recurrenceMode == ADCBuf_RECURRENCE_MODE_CONTINUOUS), NULL);

        /* Create a semaphore to block task execution for the duration of the ADC conversions */
        Semaphore_Params_init(&paramsUnion.semParams);
        paramsUnion.semParams.mode = Semaphore_Mode_BINARY;
        Semaphore_construct(&(object->conversionComplete), 0, &paramsUnion.semParams);

        /* Store internal callback function */
        object->callbackFxn = ADCBufCC26XX_conversionCallback;
    }
    else {
        /* Callback mode without a callback function defined */
        Assert_isTrue(params->callbackFxn, NULL);

        /* Save the callback function pointer */
        object->callbackFxn = params->callbackFxn;
    }

    /* Initialise the ADC object */
    /* Initialise params section of object */
    object->conversionInProgress        = false;
    object->semaphoreTimeout            = params->blockingTimeout;
    object->samplingFrequency           = params->samplingFrequency;
    object->returnMode                  = params->returnMode;
    object->recurrenceMode              = params->recurrenceMode;
    object->keepADCSemaphore            = false;
    object->adcSemaphoreInPossession    = false;

    if (params->custom) {
        /* If CC26XX specific params were specified, use them */
        object->samplingDuration        = ((ADCBufCC26XX_ParamsExtension *)(params->custom))->samplingDuration;
        object->refSource               = ((ADCBufCC26XX_ParamsExtension *)(params->custom))->refSource;
        object->samplingMode            = ((ADCBufCC26XX_ParamsExtension *)(params->custom))->samplingMode;
        object->inputScalingEnabled     = ((ADCBufCC26XX_ParamsExtension *)(params->custom))->inputScalingEnabled;
    }
    else {
        /* Initialise CC26XX specific settings to defaults */
        object->inputScalingEnabled         = true;
        object->refSource                   = ADCBufCC26XX_FIXED_REFERENCE;
        object->samplingMode                = ADCBufCC26XX_SAMPING_MODE_SYNCHRONOUS;
        object->samplingDuration            = ADCBufCC26XX_SAMPLING_DURATION_2P7_US;
    }

    /* Create the Hwi for this ADC peripheral. */
    Hwi_Params_init(&paramsUnion.hwiParams);
    paramsUnion.hwiParams.arg = (UArg) handle;
    paramsUnion.hwiParams.priority = hwAttrs->intPriority;
    Hwi_construct(&(object->hwi), INT_AUX_ADC_IRQ, ADCBufCC26XX_hwiFxn, &paramsUnion.hwiParams, NULL);

    /* Create the Swi object for this ADC peripheral */
    Swi_Params_init(&paramsUnion.swiParams);
    paramsUnion.swiParams.arg0 = (UArg)handle;
    paramsUnion.swiParams.priority = hwAttrs->swiPriority;
    Swi_construct(&(object->swi), ADCBufCC26XX_swiFxn, &(paramsUnion.swiParams), NULL);

    /* Open timer resource */
    GPTimerCC26XX_Params_init(&paramsUnion.timerParams);
    paramsUnion.timerParams.width           = GPT_CONFIG_16BIT;
    paramsUnion.timerParams.mode            = GPT_MODE_PERIODIC_UP;
    paramsUnion.timerParams.debugStallMode  = GPTimerCC26XX_DEBUG_STALL_OFF;
    object->timerHandle                     = GPTimerCC26XX_open(hwAttrs->gpTimerUnit, &paramsUnion.timerParams);

    adcPeriodCounts = ADCBufCC26XX_freqToCounts(object->samplingFrequency);
    GPTimerCC26XX_setLoadValue(object->timerHandle, adcPeriodCounts);

    GPTimerCC26XX_enableInterrupt(object->timerHandle, GPT_INT_TIMEOUT);

    /* Declare the dependency on the UDMA driver */
    object->udmaHandle = UDMACC26XX_open();

    /* Return the handle after finishing initialisation of the driver */
    Log_print0(Diags_USER1, "ADC: opened");
    return handle;

}


/*!
 *  @brief  HWI ISR of the ADC triggered when the DMA transaction is complete
 *
 *  @param  arg         An ADCBufCC26XX_Handle
 *
 */
static void ADCBufCC26XX_hwiFxn (UArg arg) {
    ADCBufCC26XX_Object            *object;
    ADCBufCC26XX_HWAttrs const     *hwAttrs;
    uint32_t                        intStatus;

    /* Get the pointer to the object and hwAttrs */
    object = ((ADCBuf_Handle)arg)->object;
    hwAttrs = ((ADCBuf_Handle)arg)->hwAttrs;

    if (object->recurrenceMode == ADCBuf_RECURRENCE_MODE_ONE_SHOT) {
        /* Disable the ADC */
        AUXADCDisable();
        /* Disable ADC DMA if we are only doing one conversion and clear DMA done interrupt. */
        HWREG(AUX_EVCTL_BASE + AUX_EVCTL_O_DMACTL) = AUX_EVCTL_DMACTL_REQ_MODE_SINGLE | AUX_EVCTL_DMACTL_SEL_FIFO_NOT_EMPTY ;
    }
    UDMACC26XX_clearInterrupt(object->udmaHandle, (1 << UDMA_CHAN_AUX_ADC) | (hwAttrs->gptDMAChannelMask));

    /* Get the status of the ADC_IRQ line and ADC_DONE */
    intStatus = HWREG(AUX_EVCTL_BASE + AUX_EVCTL_O_EVTOMCUFLAGS) & (AUX_EVCTL_EVTOMCUFLAGS_ADC_IRQ | AUX_EVCTL_EVTOMCUFLAGS_ADC_DONE);
    /* Clear the ADC_IRQ flag if it triggered the ISR */
    HWREG(AUX_EVCTL_BASE + AUX_EVCTL_O_EVTOMCUFLAGSCLR) = intStatus;

    /* Post SWI to handle remaining clean up and invocation of callback */
    Swi_post(Swi_handle(&(object->swi)));
}

/*!
 *  @brief  SWI ISR of the ADC triggered when the DMA transaction is complete
 *
 *  @param  arg0        An ADCBufCC26XX_Handle
 *
 */
static void ADCBufCC26XX_swiFxn (UArg arg0, UArg arg1) {
    ADCBuf_Conversion               *conversion;
    ADCBufCC26XX_Object             *object;
    uint16_t                        *activeSampleBuffer;

    /* Get the pointer to the object */
    object = ((ADCBuf_Handle)arg0)->object;

    Log_print0(Diags_USER2, "ADC: swi interrupt context start");

    /* Use a temporary transaction pointer in case the callback function
     * attempts to perform another ADCBuf_transfer call
     */
    conversion = object->currentConversion;

    /* Pass the primary sampleBuffer to the callback fxn by default */
    activeSampleBuffer = conversion->sampleBuffer;

    if (object->recurrenceMode == ADCBuf_RECURRENCE_MODE_ONE_SHOT) {
        /* Clean up ADC and DMA */
        ADCBufCC26XX_cleanADC(((ADCBuf_Handle)arg0));
        /* Indicate we are done with this transfer */
        object->currentConversion = NULL;
    }
    else {
        /* Reload the finished DMA control table entry */
        // TODO: switch this to a base addr relative to the hwAttrs
        if (HWREG(UDMA0_BASE + UDMA_O_SETCHNLPRIALT) & (1 << UDMA_CHAN_AUX_ADC)) {
            /* We are currently using the alternate entry -> we just finished the primary entry -> reload primary entry */
            ADCBufCC26XX_loadDMAControlTableEntry((ADCBuf_Handle)arg0, conversion, true);
            ADCBufCC26XX_loadGPTDMAControlTableEntry((ADCBuf_Handle)arg0, conversion, true);
        }
        else {
            /* We are currently using the primary entry -> we just finished the alternate entry -> reload the alternate entry */
            ADCBufCC26XX_loadDMAControlTableEntry((ADCBuf_Handle)arg0, conversion, false);
            ADCBufCC26XX_loadGPTDMAControlTableEntry((ADCBuf_Handle)arg0, conversion, false);
            activeSampleBuffer = conversion->sampleBufferTwo;
        }
    }


    /* Perform callback */
    object->callbackFxn((ADCBuf_Handle)arg0, conversion, activeSampleBuffer, object->currentChannel);

    Log_print0(Diags_USER2, "ADC: swi interrupt context end");
}

/*!
 *  @brief  CC26XX internal callback function that posts the semaphore in blocking mode
 *
 *  @param  handle      An ADCBufCC26XX_Handle
 *
 *  @param  conversion  A pointer to the current ADCBuf_Conversion
 *
 */
static void ADCBufCC26XX_conversionCallback(ADCBuf_Handle handle, ADCBuf_Conversion *conversion, void *completedADCBuffer, uint32_t completedChannel) {
    ADCBufCC26XX_Object        *object;

    Log_print0(Diags_USER1, "ADC DMA: posting conversionComplete semaphore");

    /* Get the pointer to the object */
    object = handle->object;

    /* Post the semaphore */
    Semaphore_post(Semaphore_handle(&(object->conversionComplete)));
}

/*!
 *  @brief  Function to perform ADC conversions
 *
 *  Function to perform ADC conversions, with the details passed in the
 *  ADCBufCC26XX_Conversion parameter.
 *
 *  @pre    ADCBufCC26XX_open() has been called
 *
 *  @param  handle          An ADCBufCC26XX_Handle
 *
 *  @param  conversions     Pointer to an array of conversion structures containing the
 *                          details of the ADC conversion such as DIO number
 *                          and number of samples to average for each channel.
 *
 *  @param  channelCount    Number of channels to use in this conversion. It represents the number
 *                          of ADCBuf_Conversion structs in the conversions array.
 *
 *  @return false if an error occured, check ADCBufCC26XX_Conversion.status field for details.
 *          true if conversion was successful (or in ::ADCBuf_RETURN_MODE_CALLBACK)
 *
 *  @sa     ADCBufCC26XX_open()
 *  @sa     ADCBufCC26XX_close()
 */
int_fast16_t ADCBufCC26XX_convert(ADCBuf_Handle handle, ADCBuf_Conversion conversions[],  uint_fast8_t channelCount) {
    ADCBufCC26XX_Object             *object;
    ADCBufCC26XX_HWAttrs const      *hwAttrs;
    PIN_Config                      adcPinTable[2];
    uint8_t i = 0;

    Assert_isTrue(handle, NULL);

    /* Get the pointer to the object */
    object = handle->object;
    hwAttrs = handle->hwAttrs;

    Assert_isTrue(channelCount == 1, NULL);
    Assert_isTrue((conversions->samplesRequestedCount <= UDMA_XFER_SIZE_MAX), NULL);
    Assert_isTrue(conversions->sampleBuffer, NULL);
    Assert_isTrue(!(object->recurrenceMode == (ADCBuf_RECURRENCE_MODE_CONTINUOUS && !(conversions->sampleBufferTwo))), NULL);

    /* Check if ADC is open and that no other transfer is in progress */
    uint32_t key = Hwi_disable();
    if (!(object->isOpen) || object->conversionInProgress) {

        Hwi_restore(key);
        Log_error0("ADCBuf: conversion failed");
        return ADCBuf_STATUS_ERROR;
    }
    object->conversionInProgress = true;
    Hwi_restore(key);

    /* Specify input in ADC module */
    AUXADCSelectInput(hwAttrs->adcChannelLut[conversions->adcChannel].compBInput);

    /* Add pin to measure on */
    adcPinTable[i++] = (hwAttrs->adcChannelLut[conversions->adcChannel].dio) | PIN_INPUT_EN;
    /* Terminate pin list */
    adcPinTable[i] = PIN_TERMINATE;
    object->pinHandle = PIN_open(&object->pinState, adcPinTable);
    if (!object->pinHandle) {
        object->conversionInProgress = false;
        return ADCBuf_STATUS_ERROR;
    }

    /* Save which channel we are converting on for the callbackFxn */
    object->currentChannel = conversions->adcChannel;

    /* Try to acquire the ADC semaphore if we do not already have it. */
    if (object->adcSemaphoreInPossession == false) {
         if (!AUXSMPHTryAcquire(AUX_SMPH_2)) {
            PIN_close(object->pinHandle);
            object->conversionInProgress = false;
            Log_error0("ADCBuf: failed to acquire semaphore");
            return ADCBuf_STATUS_ERROR;
        }
        object->adcSemaphoreInPossession = true;
    }

    /* Store location of the current conversion */
    object->currentConversion = conversions;

    /* Configure and arm the DMA and AUX DMA control */
    ADCBufCC26XX_configDMA(handle, conversions);

    /* Configure and arm the GPT DMA channel to clear the level-based GPT IRQ signal */
    ADCBufCC26XX_configGPTDMA(handle, conversions);

    /* Flush the ADC FIFO in case we have triggered prior to this call */
    AUXADCFlushFifo();

    /* If input scaling is set to disabled in the params, disable it */
    if (!object->inputScalingEnabled) {
        AUXADCDisableInputScaling();
    }

    /* Arm the ADC in preparation for incoming conversion triggers */
    if (object->samplingMode == ADCBufCC26XX_SAMPING_MODE_SYNCHRONOUS) {
        /* ADCBufCC26XX_SYNCHRONOUS sampling mode */
        AUXADCEnableSync(object->refSource, object->samplingDuration, AUXADC_TRIGGER_GPT0A);
    }
    else {
        /* ADCBufCC26XX_ASYNCHRONOUS sampling mode */
        AUXADCEnableAsync(object->refSource, AUXADC_TRIGGER_GPT0A);
    }

    /* Start the GPTimer to create ADC triggers */
    GPTimerCC26XX_start(object->timerHandle);

    /* Set constraints to guarantee operation */
    Power_setConstraint(PowerCC26XX_SB_DISALLOW);

    if (object->returnMode == ADCBuf_RETURN_MODE_BLOCKING) {
        Log_print0(Diags_USER1, "ADCBuf: transfer pending on conversionComplete "
                                "semaphore");

        if (!Semaphore_pend(Semaphore_handle(&(object->conversionComplete)), object->semaphoreTimeout)) {
            /* Cancel the transfer if we experience a timeout */
            ADCBufCC26XX_convertCancel(handle);
            /*
             * ADCBufCC26XX_convertCancel peforms a callback which posts a
             * conversionComplete semaphore. This call consumes this extra post.
             */
            Semaphore_pend(Semaphore_handle(&(object->conversionComplete)),
                           BIOS_NO_WAIT);
            return ADCBuf_STATUS_ERROR;
        }
    }


    return ADCBuf_STATUS_SUCCESS;
}


/*!
 *  @brief  Function that cancels an ADCBufCC26XX_convert function call.
 *
 *  This function cancels an ADCBufCC26XX_convert() operation when in
 *  ADCBufCC26XX_MODE_CALLBACK.
 *
 *  @param  handle      An ADCBufCC26XX_Handle
 */
int_fast16_t ADCBufCC26XX_convertCancel(ADCBuf_Handle handle) {
    ADCBufCC26XX_Object            *object;
    ADCBuf_Conversion              *conversion;

    Assert_isTrue(handle, NULL);

    /* Get the pointer to the object and hwAttrs*/
    object = handle->object;

    /* Check if ADC is open and that no other transfer is in progress */
    if (!(object->conversionInProgress)) {
        Log_error0("ADCBuf: a conversion must be in progress to cancel one");
        return ADCBuf_STATUS_ERROR;
    }

    /* Stop triggering a conversion on trigger events */
    AUXADCDisable();

    /* Set hardware and software configuration to default and turn off driver */
    ADCBufCC26XX_cleanADC(handle);

    /* Use a temporary transaction pointer in case the callback function
     * attempts to perform another ADCBuf_convert call
     */
    conversion = object->currentConversion;

    /* Perform callback if we are in one-shot mode. In continuous mode, ADCBuf_convertCancel will probably be called from the callback functon itself. No need to call it again. */
    if (object->recurrenceMode == ADCBuf_RECURRENCE_MODE_ONE_SHOT) {
        object->callbackFxn(handle, conversion, conversion->sampleBuffer, object->currentChannel);
    }

    return ADCBuf_STATUS_SUCCESS;
}


/*!
 *  @brief  Function to close an ADCBufCC26XX driver
 *
 *  @pre    ADCBufCC26XX_open() has to be called first.
 *
 *  @param  handle An ADCBufCC26XX handle returned from ADCBufCC26XX_open()
 *
 *  @sa     ADCBufCC26XX_open()
 */
void ADCBufCC26XX_close(ADCBuf_Handle handle) {
    ADCBufCC26XX_Object         *object;

    Assert_isTrue(handle, NULL);

    /* Get the pointer to the object */
    object = handle->object;

    /* Check if the ADC is running and abort conversion if necessary. */
    if (object->conversionInProgress) {
       ADCBuf_convertCancel(handle);
    }

    /* Get the pointer to the object */
    object = handle->object;

    /* Release the uDMA dependency and potentially power down uDMA. */
    UDMACC26XX_close(object->udmaHandle);

    /* Destroy the Hwi */
    Hwi_destruct(&(object->hwi));

    /* Destroy the Swi */
    Swi_destruct(&(object->swi));

    /* Close the timer */
    GPTimerCC26XX_close(object->timerHandle);

    if (object->returnMode == ADCBuf_RETURN_MODE_BLOCKING) {
        Semaphore_destruct(&(object->conversionComplete));
    }

    /* Mark the module as available */
    object->isOpen = false;

    Log_print0(Diags_USER1, "ADCBuf: closed");
}

/*!
 *  @brief  Function to get the resolution of a CC26XX ADC peripheral
 *
 *  @pre    ADCBufCC26XX_open() has to be called first.
 *
 *  @param  handle An ADCBufCC26XX handle returned from ADCBufCC26XX_open()
 *
 *  @return The resolution of the specified ADC in bits
 */
uint_fast8_t ADCBufCC26XX_getResolution(ADCBuf_Handle handle) {
    return (ADCBufCC26XX_RESOLUTION);
}


/*!
 *  @brief  This function adjusts a raw ADC output buffer such that the result is comparable between devices of the same make.
 *          The function does the adjustment in-place.
 *
 *  @param  handle          An ADCBuf handle returned from ADCBuf_open().
 *
 *  @param  sampleBuffer    A buffer full of raw sample values.
 *
 *  @param  sampleCount     The number of samples to adjust.
 *
 *  @param  adcChannel      Virtual channel the raw ADC values stem from. Not needed on CC26XX/CC13XX.
 *
 *  @pre    ADCBuf_open() must have been called prior.
 */
int_fast16_t ADCBufCC26XX_adjustRawValues(ADCBuf_Handle handle, void *sampleBuffer, uint_fast16_t sampleCount, uint32_t adcChannel) {
    ADCBufCC26XX_Object *object;
    uint32_t            gain;
    uint32_t            offset;
    uint16_t            i;

    object = handle->object;

    gain = AUXADCGetAdjustmentGain(object->refSource);
    offset = AUXADCGetAdjustmentOffset(object->refSource);

    for (i = 0; i < sampleCount; i++) {
        uint16_t tmpRawADCVal = ((uint16_t *)sampleBuffer)[i];
        ((uint16_t *)sampleBuffer)[i] = AUXADCAdjustValueForGainAndOffset(tmpRawADCVal, gain, offset);
    }

    return ADCBuf_STATUS_SUCCESS;
}

/*!
 *  @brief  This function converts a raw ADC output value to a value scaled in micro volts.
 *
 *  @param  handle      An ADCBuf handle returned from ADCBuf_open()
 *
 *  @param  adcChannel  This parameter is irrelevant for the CC26XX implementation.
 *
 *  @param  adjustedSampleBuffer    A buffer full of adjusted samples.
 *
 *  @param  outputMicroVoltBuffer   The output buffer. The conversion does not occur in place due to the differing data type sizes.
 *
 *  @param  sampleCount             The number of samples to convert.
 *
 *  @return A measurement scaled in micro volts.
 *
 *  @pre    ADCBuf_open() must have been called prior.
 *
 *  @pre    ADCBuf_adjustRawValues() must be called on adjustedSampleBuffer prior.
 */
int_fast16_t ADCBufCC26XX_convertAdjustedToMicroVolts(ADCBuf_Handle handle, uint32_t  adcChannel, void *adjustedSampleBuffer, uint32_t outputMicroVoltBuffer[], uint_fast16_t sampleCount) {
    ADCBufCC26XX_Object *object;
    uint16_t            i;
    uint32_t            voltageRef;

    object = handle->object;

    voltageRef = (object->inputScalingEnabled) ? AUXADC_FIXED_REF_VOLTAGE_NORMAL : AUXADC_FIXED_REF_VOLTAGE_UNSCALED;

    for (i = 0; i < sampleCount; i++) {
        outputMicroVoltBuffer[i] = AUXADCValueToMicrovolts(voltageRef, ((uint16_t *)adjustedSampleBuffer)[i]);
    }

    return ADCBuf_STATUS_SUCCESS;
}

/*!
 *  @brief  Function to configure the DMA to automatically transfer ADC output data into a provided array
 *
 *  @pre    ADCBufCC26XX_open() has to be called first.
 *
 *  @pre    There must not currently be a conversion in progress
 *
 *  @param  handle An ADCBufCC26XX handle returned from ADCBufCC26XX_open()
 *
 *  @param  conversion A pointer to an ADCBuf_Conversion
 *
 */
static void ADCBufCC26XX_configDMA(ADCBuf_Handle handle, ADCBuf_Conversion *conversion) {
    ADCBufCC26XX_Object             *object;

    /* Get the pointer to the object */
    object = handle->object;

    /* Set configure control table entry */
    ADCBufCC26XX_loadDMAControlTableEntry(handle, conversion, true);

    /* If we are operating in continous mode, load the alternate DMA control table data structure */
    if (object->recurrenceMode == ADCBuf_RECURRENCE_MODE_CONTINUOUS) {
        ADCBufCC26XX_loadDMAControlTableEntry(handle, conversion, false);
    }

    /* Enable the channels */
    UDMACC26XX_channelEnable(object->udmaHandle, 1 << UDMA_CHAN_AUX_ADC);

    /* Configure DMA settings in AUX_EVCTL */
    HWREG(AUX_EVCTL_BASE + AUX_EVCTL_O_DMACTL) = AUX_EVCTL_DMACTL_REQ_MODE_SINGLE | AUX_EVCTL_DMACTL_EN | AUX_EVCTL_DMACTL_SEL_FIFO_NOT_EMPTY;

    Log_print0(Diags_USER1,"ADCBuf: DMA transfer enabled");
}

/*!
 *  @brief  Function to configure the adc DMA control table entry for basic or ping pong mode
 *
 *  @pre    ADCBufCC26XX_open() has to be called first.
 *
 *  @pre    There must not currently be a conversion in progress
 *
 *  @param  handle An ADCBufCC26XX handle returned from ADCBufCC26XX_open()
 *
 *  @param  conversion A pointer to an ADCBuf_Conversion
 *
 *  @param  primaryEntry Is this supposed to modify the primary or the alternate control table entry
 *
 */
static void ADCBufCC26XX_loadDMAControlTableEntry(ADCBuf_Handle handle, ADCBuf_Conversion *conversion, bool primaryEntry) {
    ADCBufCC26XX_Object             *object;
    volatile tDMAControlTable       *dmaControlTableEntry;
    uint32_t                        numberOfBytes;

    /* Get the pointer to the object*/
    object = handle->object;

    /* Calculate the number of bytes for the transfer */
    numberOfBytes = (uint16_t)(conversion->samplesRequestedCount) * ADCBufCC26XX_BYTES_PER_SAMPLE;

    /* Set configure control table entry */
    dmaControlTableEntry = primaryEntry ? &dmaADCPriControlTableEntry : &dmaADCAltControlTableEntry;
    dmaControlTableEntry->ui32Control = ((object->recurrenceMode == ADCBuf_RECURRENCE_MODE_ONE_SHOT) ? UDMA_MODE_BASIC : UDMA_MODE_PINGPONG) |
                                        UDMA_SIZE_16  |
                                        UDMA_SRC_INC_NONE |
                                        UDMA_DST_INC_16 |
                                        UDMA_ARB_1 |
                                        UDMACC26XX_SET_TRANSFER_SIZE((uint16_t)conversion->samplesRequestedCount);
    dmaControlTableEntry->pvDstEndAddr = (void *)((uint32_t)(primaryEntry ? conversion->sampleBuffer : conversion->sampleBufferTwo) + numberOfBytes - 1);
    dmaControlTableEntry->pvSrcEndAddr = (void *)(AUX_ANAIF_BASE + AUX_ANAIF_O_ADCFIFO);
}

/*!
 *  @brief  Function to configure the DMA to automatically clear the GPT_IRQ line that the ADC triggers off of without using an interrupt handler
 *
 *  @pre    ADCBufCC26XX_open() has to be called first.
 *
 *  @pre    There must not currently be a conversion in progress
 *
 *  @param  handle An ADCBufCC26XX handle returned from ADCBufCC26XX_open()
 *
 *  @param  conversion A pointer to an ADCBuf_Conversion
 *
 */
static void ADCBufCC26XX_configGPTDMA(ADCBuf_Handle handle, ADCBuf_Conversion *conversion) {
    ADCBufCC26XX_Object             *object;
    ADCBufCC26XX_HWAttrs const      *hwAttrs;

    /* Get the pointer to the object and hwAttrs */
    object = handle->object;
    hwAttrs = handle->hwAttrs;

    /* Set configure control table entry */
    ADCBufCC26XX_loadGPTDMAControlTableEntry(handle, conversion, true);

    /* If we are operating in continous mode, load the alternate DMA control table data structure */
    if (object->recurrenceMode == ADCBuf_RECURRENCE_MODE_CONTINUOUS) {
        ADCBufCC26XX_loadGPTDMAControlTableEntry(handle, conversion, false);
    }

    /* Enable the channels */
    UDMACC26XX_channelEnable(object->udmaHandle, hwAttrs->gptDMAChannelMask);

    /* Enable event signal */
    HWREG(object->timerHandle->hwAttrs->baseAddr + GPT_O_DMAEV) = GPT_DMAEV_TATODMAEN;

}

/*!
 *  @brief  Function to configure the GPT DMA control table entry for basic or ping pong mode
 *
 *  @pre    ADCBufCC26XX_open() has to be called first.
 *
 *  @pre    There must not currently be a conversion in progress
 *
 *  @param  handle An ADCBufCC26XX handle returned from ADCBufCC26XX_open()
 *
 *  @param  conversion A pointer to an ADCBuf_Conversion
 *
 *  @param  primaryEntry Is this supposed to modify the primary or the alternate control table entry
 *
 */
static void ADCBufCC26XX_loadGPTDMAControlTableEntry(ADCBuf_Handle handle, ADCBuf_Conversion *conversion, bool primaryEntry) {
    ADCBufCC26XX_Object             *object;
    volatile tDMAControlTable       *dmaControlTableEntry;

    /* Get the pointer to the object */
    object = handle->object;

    /* Set configure control table entry */
    dmaControlTableEntry = primaryEntry ? &dmaGPT0APriControlTableEntry : &dmaGPT0AAltControlTableEntry;
    dmaControlTableEntry->ui32Control = ((object->recurrenceMode == ADCBuf_RECURRENCE_MODE_ONE_SHOT) ? UDMA_MODE_BASIC : UDMA_MODE_PINGPONG) |
                                        UDMA_SIZE_8  |
                                        UDMA_SRC_INC_NONE |
                                        UDMA_DST_INC_NONE |
                                        UDMA_ARB_1 |
                                        UDMACC26XX_SET_TRANSFER_SIZE((uint16_t)conversion->samplesRequestedCount);
    dmaControlTableEntry->pvDstEndAddr = (void *)((uint32_t)(object->timerHandle->hwAttrs->baseAddr + GPT_O_ICLR));
    dmaControlTableEntry->pvSrcEndAddr = (void *)(&gptClear);
}

/*!
 *  @brief  Function to undo all configurations done by the ADC driver
 *
 *  @pre    ADCBuf_open() has to be called first.
 *
 *  @pre    ADCBuf_convert() has to be called first.
 *
 *  @param  handle An ADCBufCC26XX handle returned from ADCBufCC26XX_open()
 *
 */
static void ADCBufCC26XX_cleanADC(ADCBuf_Handle handle) {
    ADCBufCC26XX_Object            *object;
    ADCBufCC26XX_HWAttrs const     *hwAttrs;

    /* Get the pointer to the object and hwAttrs */
    object = handle->object;
    hwAttrs = handle->hwAttrs;

    /* Stop the timer to stop generating triggers */
    GPTimerCC26XX_stop(object->timerHandle);

    /* Set constraints to guarantee operation */
    Power_releaseConstraint(PowerCC26XX_SB_DISALLOW);

    if (object->adcSemaphoreInPossession && !object->keepADCSemaphore) {
        /* Release the ADC semaphore */
        AUXSMPHRelease(AUX_SMPH_2);
        object->adcSemaphoreInPossession = false;
    }

    /* Disable the UDMA channels */
    UDMACC26XX_channelDisable(object->udmaHandle, (1 << UDMA_CHAN_AUX_ADC) | (hwAttrs->gptDMAChannelMask));

    /* Deallocate pins */
    PIN_close(object->pinHandle);

    /* Disable UDMA mode for ADC */
    HWREG(AUX_EVCTL_BASE + AUX_EVCTL_O_DMACTL) = AUX_EVCTL_DMACTL_REQ_MODE_SINGLE | AUX_EVCTL_DMACTL_SEL_FIFO_NOT_EMPTY ;

    /* Clear any remaining GPT_IRQ flags */
    HWREG(object->timerHandle->hwAttrs->baseAddr + GPT_O_ICLR) = GPT_ICLR_DMAAINT + GPT_ICLR_TATOCINT;

    /* Note that the driver is no longer converting */
    object->conversionInProgress = false;
}

/* Return period in timer counts */
static uint32_t ADCBufCC26XX_freqToCounts(uint32_t frequency)
{
    Types_FreqHz freq;
    BIOS_getCpuFreq(&freq);

    uint32_t periodCounts = (freq.lo / frequency) - 1;

    return periodCounts;
}

/*!
 *  @brief  Function to acquire the semaphore that arbitrates access to the ADC
 *          between the CM3 and the sensor controller
 *
 *  @pre    ADCBufCC26XX_open() has to be called first.
 *
 *  @pre    There must not currently be a conversion in progress
 *
 *  @param  handle An ADCBufCC26XX handle returned from ADCBufCC26XX_open()
 *
 */
static bool ADCBufCC26XX_acquireADCSemaphore(ADCBuf_Handle handle) {
    ADCBufCC26XX_Object         *object;

    object = handle->object;

    /* Check if ADC is closed or a conversion is in progress */
    uint32_t key = Hwi_disable();
    if (!(object->isOpen) || object->conversionInProgress) {
        Hwi_restore(key);
        Log_error0("ADC: driver must be open and no conversion must be in progress to disable input scaling");
        return false;
    }
    /* This is a non-blocking call to acquire the ADC semaphore. */
    if (AUXSMPHTryAcquire(AUX_SMPH_2)) {
        object->adcSemaphoreInPossession = true;
        Hwi_restore(key);
        return true;
    }
    Hwi_restore(key);
    return false;
}

/*!
 *  @brief  This function releases the ADC semaphore
 *
 *  @pre    ADCBufCC26XX_open() has to be called first.
 *
 *  @pre    There must not currently be a conversion in progress
 *
 *  @param  handle An ADCBufCC26XX handle returned from ADCBufCC26XX_open()
 *
 */
static bool ADCBufCC26XX_releaseADCSemaphore(ADCBuf_Handle handle) {
    ADCBufCC26XX_Object *object;

    object= handle->object;

    /* Check if ADC is closed or a conversion is in progress */
    uint32_t key = Hwi_disable();
    if (!(object->isOpen) || object->conversionInProgress) {
        Hwi_restore(key);
        Log_error0("ADC: driver must be open and no conversion must be in progress to disable input scaling");
        return false;
    }
    /* Release the ADC semaphore */
    AUXSMPHRelease(AUX_SMPH_2);
    object->adcSemaphoreInPossession = false;
    Hwi_restore(key);

    return true;
}

/*!
 *  @brief  Function to implement CC26XX specific ADC functions
 *
 *  @pre    ADCBufCC26XX_open() has to be called first.
 *
 *  @param  handle An ADCBufCC26XX handle returned from ADCBufCC26XX_open()
 *
 *  @param  cmd Value to specify which command to execute
 *
 *  @param  arg Pointer to input arguments for the control command to be executed
 *
 */
int_fast16_t ADCBufCC26XX_control(ADCBuf_Handle handle, uint_fast16_t cmd, void * arg) {
    ADCBufCC26XX_Object *object = handle->object;
    int status = ADCBuf_STATUS_ERROR;

    Assert_isTrue(handle, NULL);

    switch (cmd) {
        case ADCBufCC26XX_CMD_ACQUIRE_ADC_SEMAPHORE:
            if (ADCBufCC26XX_acquireADCSemaphore(handle)) {
                status = ADCBuf_STATUS_SUCCESS;
            }
            break;
        case ADCBufCC26XX_CMD_KEEP_ADC_SEMAPHORE:
            object->keepADCSemaphore = true;
            status = ADCBuf_STATUS_SUCCESS;
            break;
        case ADCBufCC26XX_CMD_KEEP_ADC_SEMAPHORE_DISABLE:
            object->keepADCSemaphore = false;
            status = ADCBuf_STATUS_SUCCESS;
            break;
        case ADCBufCC26XX_CMD_RELEASE_ADC_SEMAPHORE:
            if (ADCBufCC26XX_releaseADCSemaphore(handle)) {
                status = ADCBuf_STATUS_SUCCESS;
            }
            break;
        default:
            status = ADCBuf_STATUS_UNDEFINEDCMD;
            break;
    }
    return status;
}
