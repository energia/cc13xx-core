/*
 * Copyright (c) 2015, Texas Instruments Incorporated
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

#define DEVICE_FAMILY cc13x0

#include <ti/runtime/wiring/Energia.h>
#include <ti/sysbios/family/arm/m3/TimestampProvider.h>
#include <ti/sysbios/family/arm/m3/Hwi.h>
#include <xdc/runtime/Types.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>

/*
 *  ======== micros ========
 */
unsigned long micros(void)
{
    Types_FreqHz freq;
    Types_Timestamp64 time;
    uint64_t t64;

    TimestampProvider_getFreq(&freq);
    TimestampProvider_get64(&time);
    t64 = ((uint64_t)time.hi << 32) | time.lo;
    return (t64/(freq.lo/1000000));
}

/*
 *  ======== millis ========
 */
unsigned long millis(void)
{
    uint64_t milliseconds;

    milliseconds = ((uint64_t)Clock_getTicks() * (uint64_t)Clock_tickPeriod) / (uint64_t)1000;

    return (milliseconds);
}

/*
 *  ======== delayMicroseconds ========
 *  Delay for the given number of microseconds
 */
void delayMicroseconds(unsigned int us)
{
    if (us <7) {
        //The overhead in calling and returning from this function takes about 6us
    }
    else if (us <=20) {
        int time;
        for (time = 5*(us-6); time > 0; time--) {
            asm("   nop");
        }
    }
    else if (us < 70) {
        int time;
        for (time = 5*us; time > 0; time--) {
            asm("   nop");
        }
    }
    else {
        uint32_t t0, deltaT;
        Types_FreqHz freq;

        TimestampProvider_getFreq(&freq);
        deltaT = us * (freq.lo/1000000);

        t0 = TimestampProvider_get32();

        while ((TimestampProvider_get32()-t0) < deltaT) {
            ;
        }
    }
}

/*
 *  ======== delay ========
 */
void delay(uint32_t milliseconds)
{
    uint64_t delay;

    if (milliseconds == 0) {
        Task_yield();
        return;
    }

    /* math below assumes tickPeriod is <= 1000 */
    delay = ((uint64_t)milliseconds * (uint64_t)1000) / (uint64_t)Clock_tickPeriod;

    Task_sleep(delay);
}

/*
 * The following is a hack to force the usage of our local modified
 * copy of UARTCC26XX.c to support the PEEK and AVAILABLE control
 * commands. These control commands will be available in the
 * 3.10.00 TI Drivers.
 */

#include <ti/drivers/UART.h>
#include <ti/drivers/uart/UARTCC26XX.h>

const UART_FxnTable *uartFxnTablePtr = &UARTCC26XX_fxnTable;

/*
 * The following two functions implement a hack to
 * force the Clock functions to be executed within
 * the Clock timer interrupt thread rather than
 * within a Clock Swi so that UART_write() can be called within
 * an ISR thread.
 * Without this hack, the tx complete callback is called from within
 * a Swi context and Swis are only executed after all Hwis are finished.
 * Consequently, the UART_write() callback will never get called while
 * the current thread is an ISR.
 */
void myClock_doTick(UArg arg) {
    Clock_workFuncDynamic(0,0);
}

/*
 * reroute cc26xx Timer func to "myClock_doTick"
 * Make cc26xx Timer interrupt higher priority than GPIO interrupts
 * so that they will nest user "attachInterrupt()" pin interrupts.
 */
void energiaLastFxn() {
    Hwi_setPriority(20, 0xc0);
    Clock_TimerProxy_setFunc(Clock_getTimerHandle(), myClock_doTick, 0);
}


