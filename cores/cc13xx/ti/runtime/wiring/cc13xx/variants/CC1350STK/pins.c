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

#include <ti/runtime/wiring/wiring_private.h>
#include <ti/runtime/wiring/cc13xx/wiring_analog.h>

#include <inc/hw_types.h>
#include <inc/hw_memmap.h>

uint8_t digital_pin_to_pin_function[] = {
    PIN_FUNC_INVALID,   /*  dummy */
    PIN_FUNC_INVALID,   /*  1  - VDD */
    PIN_FUNC_INVALID,   /*  2  - GND */
    PIN_FUNC_UNUSED,    /*  3  - DIO_06 SCL */
    PIN_FUNC_UNUSED,    /*  4  - DIO_05 SDA */
    PIN_FUNC_UNUSED,    /*  5  - DIO_16 DP12/AUDIO FS/TD0 */
    PIN_FUNC_UNUSED,    /*  6  - DIO_03 DP7/AUDIO CLK */
    PIN_FUNC_UNUSED,    /*  7  - DIO_20 DP11/CSN */
    PIN_FUNC_UNUSED,    /*  8  - DIO_22 DP6/AUDIO DO */
    PIN_FUNC_INVALID,   /*  9  - VDD */
    PIN_FUNC_UNUSED,    /*  10 - DIO_29 DP5/UART_TX */
    PIN_FUNC_UNUSED,    /*  11 - DIO_19 DP10/MOSI */
    PIN_FUNC_UNUSED,    /*  12 - DIO_28 DP4/UART_RX */
    PIN_FUNC_UNUSED,    /*  13 - DIO_18 DP9/MISO */
    PIN_FUNC_UNUSED,    /*  14 - DIO_27 DP3 */
    PIN_FUNC_UNUSED,    /*  15 - DIO_17 DP8/SCLK/TDI */
    PIN_FUNC_UNUSED,    /*  16 - DIO_23 DP2  */
    PIN_FUNC_UNUSED,    /*  17 - DIO_30 DP_ID */
    PIN_FUNC_UNUSED,    /*  18 - DIO_24 DP1 */
    PIN_FUNC_INVALID,   /*  19 - POWER_GOOD */
    PIN_FUNC_UNUSED,    /*  20 - DIO_25 DP0 */
    PIN_FUNC_UNUSED,    /*  21 - DIO_02 AUDIO DI */
    PIN_FUNC_UNUSED,    /*  22 - DIO_12 MPU PWR */
    PIN_FUNC_UNUSED,    /*  23 - DIO_21 BUZZER */
    PIN_FUNC_UNUSED,    /*  24 - DIO_01 REED */
    PIN_FUNC_UNUSED,    /*  25 - DIO_04 BUTTON1 */
    PIN_FUNC_UNUSED,    /*  26 - DIO_11 TMP RDY */
    PIN_FUNC_UNUSED,    /*  27 - DIO_10 LED1 */
    PIN_FUNC_UNUSED,    /*  28 - DIO_07 MPU INT */
    PIN_FUNC_UNUSED,    /*  29 - DIO_13 MIC PWR */
    PIN_FUNC_UNUSED,    /*  30 - DIO_15 BUTTON2 */
    PIN_FUNC_UNUSED,    /*  31 - Dummy (LED2 on CC2650 STK) */
    PIN_FUNC_UNUSED,    /*  32 - DIO_14 FLASH CS */
};
