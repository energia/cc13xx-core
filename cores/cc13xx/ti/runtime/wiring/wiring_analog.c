/*
 * Copyright (c) 2017, Texas Instruments Incorporated
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

#include <ti/runtime/wiring/wiring_private.h>
#include <ti/drivers/ADC.h>

extern ADC_Config ADC_config[];

static bool adcInitialized = false;

/*
 * \brief           Reads an analog value from the pin specified.
 * \param[in] pin   The pin number to read from.
 * \return          A 16-bit integer containing a N-bit sample from the ADC.
 */
uint16_t analogRead(uint8_t pin)
{
    uint8_t adcIndex = digital_pin_to_adc_index[pin];
    uint16_t sample;

    if (adcIndex == NOT_ON_ADC) return (0);

    /* re-configure pin if necessary */
    if (digital_pin_to_pin_function[pin] != PIN_FUNC_ANALOG_INPUT) {
        ADC_Params adcParams;
        ADC_Handle adcHandle;

        /* undo pin's current plumbing */
        switch (digital_pin_to_pin_function[pin]) {
            case PIN_FUNC_ANALOG_OUTPUT:
                stopAnalogWrite(pin);
                break;
            case PIN_FUNC_DIGITAL_INPUT:
                stopDigitalRead(pin);
                break;
            case PIN_FUNC_DIGITAL_OUTPUT:
                stopDigitalWrite(pin);
                break;
        }

        if (adcInitialized == false) {
            ADC_init();
            adcInitialized = true;
        }

        ADC_Params_init(&adcParams);
        adcParams.isProtected = false;  /* do NOT use a semaphore for thread safety */

        adcHandle = ADC_open(adcIndex, &adcParams);

        if (adcHandle == NULL) {
            return (0);
        }

        digital_pin_to_pin_function[pin] = PIN_FUNC_ANALOG_INPUT;
    }

    ADC_convert((ADC_Handle)&(ADC_config[adcIndex]), &sample);

    if (analogReadShift >= 0) {
        return (sample >> analogReadShift);
    }
    else {
        return (sample << -analogReadShift);
    }
}

