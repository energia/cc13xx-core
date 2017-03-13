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

#define ARDUINO_MAIN

#define DEVICE_FAMILY cc13x0

#include <ti/runtime/wiring/wiring_private.h>
#include "wiring_analog.h"

#include <ti/drivers/PWM.h>
#include <ti/drivers/pwm/PWMTimerCC26XX.h>

#include <ti/drivers/GPIO.h>
#include <ti/drivers/gpio/GPIOCC26XX.h>

#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26XX.h>

#include <ti/drivers/PIN.h>
#include <ti/drivers/pin/PINCC26XX.h>

#include <ti/drivers/ADC.h>

#include <ti/sysbios/family/arm/m3/Hwi.h>
#include <ti/sysbios/family/arm/lm4/Timer.h>

#include <ti/devices/cc13x0/driverlib/ioc.h>
#include <ti/devices/cc13x0/driverlib/aux_adc.h>
#include <ti/devices/cc13x0/driverlib/aux_wuc.h>

/*
 * analogWrite() support
 */

extern PWM_Config PWM_config[];
extern const GPIOCC26XX_Config GPIOCC26XX_config;
extern PWMTimerCC26XX_HwAttrs pwmtimerCC26xxHWAttrs[];
extern ADC_Config ADC_config[];

/* Carefully selected hal Timer IDs for tone and servo */
uint32_t toneTimerId = (~0);  /* use Timer_ANY for tone timer */
uint32_t servoTimerId = (~0); /* use Timer_ANY for servo timer */

#define PWM_NOT_IN_USE 0

/* Current PWM timer GPIO mappings */
uint8_t used_pwm_port_pins[] = {
    PWM_NOT_IN_USE,
    PWM_NOT_IN_USE,
    PWM_NOT_IN_USE,
    PWM_NOT_IN_USE,
    PWM_NOT_IN_USE,
    PWM_NOT_IN_USE,
    PWM_NOT_IN_USE,
    PWM_NOT_IN_USE,
};

/*
 * While in use, this table contains
 * the index of the PWM Timer resource that is mapped
 * to the corresponding pin.
 */
uint16_t digital_pin_to_pwm_index[32];

/*
 * Common digital pin decommissioning function
 */
void stopDigitalIO(uint8_t pin)
{
    GPIOCC26xx_release(pin);
}

/*
 * Do whatever is necessary to prepare the digital input pin to be
 * configured in any other mode
 */
void stopDigitalRead(uint8_t pin)
{
   stopDigitalIO(pin);
}

/*
 * Do whatever is necessary to prepare the digital output pin to be
 * configured in any other mode
 */
 void stopDigitalWrite(uint8_t pin)
{
   stopDigitalIO(pin);
}

/*
 * For the CC26xx, the timers used for PWM are clocked at 48MHz.
 * A PWM period of 2.04ms is chosen for Arduino compatibility.
 * The period is set to 2.040ms in the PWM_open() calls below.
 * The PWM objects are configured for PWM_DUTY_COUNTS mode to minimize
 * the PWM_setDuty() processing overhead.
 * The 2.040ms period yields a period count of 97,920.
 * The Arduino analogWrite() API takes a value of 0-255 for the duty cycle.
 * The PWM scale factor is then 97,920 / 255 = 384
 */

#define PWM_SCALE_FACTOR 384

void analogWrite(uint8_t pin, int val)
{
    uint8_t pwmIndex;
    uint8_t pwmPinId;
    uint32_t hwiKey;

    hwiKey = Hwi_disable();

    if (digital_pin_to_pin_function[pin] == PIN_FUNC_ANALOG_OUTPUT) {
        pwmIndex = digital_pin_to_pwm_index[pin];
    }
    else {
        /* re-configure pin if possible */
        PWM_Params pwmParams;
        PWM_Handle pwmHandle;
        uint8_t numPwmChannels = sizeof(used_pwm_port_pins)/sizeof(uint8_t);

        if (digital_pin_to_pin_function[pin] == PIN_FUNC_INVALID) {
            Hwi_restore(hwiKey);
            return; /* can't get there from here */
        }

        /* extract 16bit pinID from pin */
        pwmPinId = GPIOCC26XX_config.pinConfigs[pin] & 0xff;

        /* undo pin's current plumbing */
        switch (digital_pin_to_pin_function[pin]) {
            case PIN_FUNC_ANALOG_INPUT:
                stopAnalogRead(pin);
                break;
            case PIN_FUNC_DIGITAL_INPUT:
                stopDigitalRead(pin);
                break;
            case PIN_FUNC_DIGITAL_OUTPUT:
                stopDigitalWrite(pin);
                break;
        }

        /* find an unused PWM resource and port map it */
        for (pwmIndex = 0; pwmIndex < numPwmChannels; pwmIndex++) {
            if (used_pwm_port_pins[pwmIndex] == PWM_NOT_IN_USE) {

                /* Open the PWM port */
                PWM_Params_init(&pwmParams);

                pwmParams.periodUnits = PWM_PERIOD_COUNTS;
                pwmParams.periodValue = 97920; /* arduino period is 2.04ms (490Hz) */
                pwmParams.dutyUnits = PWM_DUTY_COUNTS;

                /* override default pwmPin in HwAttrs */
                pwmtimerCC26xxHWAttrs[pwmIndex].pwmPin = pwmPinId;

                /*
                 * PWM_open() will fail if the timer's CCR is already in use or
                 * the pin has been assigned to some other function (ie UART, I2C, SPI)
                 */
                pwmHandle = PWM_open(pwmIndex, &pwmParams);

                if (pwmHandle != NULL) {
                    /* remember which pinId is being used by this PWM resource */
                    used_pwm_port_pins[pwmIndex] = pwmPinId; /* save pwm pin info */
                    /* remember which PWM resource is being used by this pin */
                    digital_pin_to_pwm_index[pin] = pwmIndex; /* save pwm index */
                    digital_pin_to_pin_function[pin] = PIN_FUNC_ANALOG_OUTPUT;
                    /* success! */
                    break;
                }
                else {
                    /* try next PWM index */
                }
            }
        }

        if (pwmIndex > (numPwmChannels-1)) {
            Hwi_restore(hwiKey);
            return; /* no available PWM ports */
        }

        /* start Timer */
        PWM_start(pwmHandle);
    }

    Hwi_restore(hwiKey);

    PWM_setDuty((PWM_Handle)&(PWM_config[pwmIndex]), (val * PWM_SCALE_FACTOR));
}

/*
 * This internal API is used to de-configure a pin that has been
 * put in analogWrite() mode.
 *
 * It will free up the pin's PWM resource after
 * it is no longer being used to support analogWrite() on a different
 * pin. It is called by pinMap() when a pin's function is being modified.
 */
void stopAnalogWrite(uint8_t pin)
{
    uint16_t pwmIndex = digital_pin_to_pwm_index[pin];
    /* Close PWM port */
    PWM_close((PWM_Handle)&(PWM_config[pwmIndex]));
    /* restore pin table entry with port/pin info */
    digital_pin_to_pwm_index[pin] = used_pwm_port_pins[pwmIndex];
    /* free up pwm resource */
    used_pwm_port_pins[pwmIndex] = PWM_NOT_IN_USE;
}

/*
 * analogRead() support
 */

int8_t analogReadShift = 2;

/*
 * \brief           configure the A/D reference voltage
 * \param mode      DEFAULT, INTERNAL, EXTERNAL, ...
 * \return          void
 */
void analogReference(uint16_t mode)
{
}

/*
 * This internal API is used to de-configure a pin that has been
 * put in analogRead() mode.
 *
 * It is called by pinMap() when a pin's function is
 * being modified.
 */
void stopAnalogRead(uint8_t pin)
{
    uint8_t adcIndex = digital_pin_to_adc_index[pin];

    /* Close PWM port */
    ADC_close((ADC_Handle)&(ADC_config[adcIndex]));

    digital_pin_to_pin_function[pin] = PIN_FUNC_UNUSED;
}

/*
 * \brief sets the number of bits to shift the value read by ADCFIFORead()
 */
void analogReadResolution(uint16_t bits)
{
    analogReadShift = 12 - bits;
}
