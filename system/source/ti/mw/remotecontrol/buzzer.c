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

/** ============================================================================
 *  @file       buzzer.c
 *
 *  @brief      PWM-based buzzer interface.
 *  ============================================================================
 */

/* -----------------------------------------------------------------------------
*  Includes
* ------------------------------------------------------------------------------
*/

// TI RTOS drivers
#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26XX.h>

// Temporary PWM solution directly on DriverLib
// (until a Timer RTOS driver is in place)
#include <ti/drivers/pin/PINCC26XX.h>
#include <driverLib/timer.h>

#include "buzzer.h"

/* -----------------------------------------------------------------------------
*  Local variables
* ------------------------------------------------------------------------------
*/
static PIN_Handle hPin = NULL;

/* -----------------------------------------------------------------------------
*  Public Functions
* ------------------------------------------------------------------------------
*/

/*******************************************************************************
 * @fn          buzzerOpen
 *
 * @brief       Initialize the Buzzer
 *
 * @descr       Initializes pin and PWM
 *
 * @return      -
 */
void buzzerOpen(PIN_Handle hGpioPin)
{
    hPin = hGpioPin;

    // Turn on PERIPH power domain and clock for GPT0 and GPIO
    Power_setDependency(PowerCC26XX_PERIPH_GPT0);
    Power_setConstraint(PowerCC26XX_SB_DISALLOW);

    // Assign GPT0
    TimerConfigure(GPT0_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PWM);

    // Configure pin for PWM output
    PINCC26XX_setMux(hPin, Board_BUZZER, IOC_PORT_MCU_PORT_EVENT0);
}


/*******************************************************************************
 * @fn          buzzerSetFrequency
 *
 * @brief       Set the frequency (3Hz - 8 KHz)
 *
 * @return      return true if the requency is within range
 */
bool buzzerSetFrequency(uint16_t freq)
{
    uint32_t ticks;
    uint32_t loadLow;
    uint32_t loadHigh;
    uint32_t matchLow;
    uint32_t matchHigh;

    if (freq < BUZZER_FREQ_MIN && freq > BUZZER_FREQ_MAX)
    {
        return false;
    }

    // Stop timer during reconfiguration
    TimerDisable(GPT0_BASE, TIMER_A);

    // Calculate timer load and match values
    ticks = 48000000 / freq;
    loadLow = ticks & 0x0000FFFF;
    loadHigh = (ticks & 0x00FF0000) >> 16;
    matchLow = (ticks / 2) & 0x0000FFFF;
    matchHigh = ((ticks / 2) & 0x00FF0000) >> 16;

    // Set timer load
    TimerLoadSet(GPT0_BASE, TIMER_A, loadLow);
    TimerPrescaleSet(GPT0_BASE, TIMER_A, loadHigh);

    // Set timer match
    TimerMatchSet(GPT0_BASE, TIMER_BOTH, matchLow);
    TimerPrescaleMatchSet(GPT0_BASE, TIMER_A, matchHigh);

    // Start timer
    TimerEnable(GPT0_BASE, TIMER_A);

    return true;
}

/*******************************************************************************
 * @fn          buzzerClose
 *
 * @brief       Closes the buzzer interface
 *
 * @return      -
 */
void buzzerClose(void)
{
    // Configure pin as GPIO
    PINCC26XX_setMux(hPin, Board_BUZZER, IOC_PORT_GPIO);

    // Turn off PERIPH power domain and clock for GPT0
    Power_releaseDependency(PowerCC26XX_PERIPH_GPT0);
    Power_releaseConstraint(PowerCC26XX_SB_DISALLOW);
}
