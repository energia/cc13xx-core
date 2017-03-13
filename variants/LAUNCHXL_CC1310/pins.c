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

#include <ti/runtime/wiring/wiring_private.h>
#include <ti/runtime/wiring/cc13xx/wiring_analog.h>

uint8_t digital_pin_to_pin_function[] = {
    PIN_FUNC_INVALID,   /*  dummy */

    PIN_FUNC_INVALID,   /*  1  - 3.3V */
    PIN_FUNC_UNUSED,    /*  2  - DIO_23 */
    PIN_FUNC_UNUSED,    /*  3  - DIO_03 */
    PIN_FUNC_UNUSED,    /*  4  - DIO_02 */
    PIN_FUNC_UNUSED,    /*  5  - DIO_22 */
    PIN_FUNC_UNUSED,    /*  6  - DIO_24 */
    PIN_FUNC_UNUSED,    /*  7  - DIO_10 */
    PIN_FUNC_UNUSED,    /*  8  - DIO_21 */
    PIN_FUNC_UNUSED,    /*  9  - DIO_04 */
    PIN_FUNC_UNUSED,    /*  10 - DIO_05 */
	
    PIN_FUNC_UNUSED,    /*  11 - DIO_15 */
    PIN_FUNC_UNUSED,    /*  12 - DIO_14 */
    PIN_FUNC_UNUSED,    /*  13 - DIO_13 */
    PIN_FUNC_UNUSED,    /*  14 - DIO_08 */
    PIN_FUNC_UNUSED,    /*  15 - DIO_09 */
    PIN_FUNC_INVALID,   /*  16 - LPRST */
    PIN_FUNC_INVALID,   /*  17 - NC */
    PIN_FUNC_UNUSED,    /*  18 - DIO_11 */
    PIN_FUNC_UNUSED,    /*  19 - DIO_12 */
    PIN_FUNC_INVALID,   /*  20 - GND */
	
    PIN_FUNC_INVALID,   /*  21 - 5V */
    PIN_FUNC_INVALID,   /*  22 - GND */
    PIN_FUNC_UNUSED,    /*  23 - DIO_25 */
    PIN_FUNC_UNUSED,    /*  24 - DIO_26 */
    PIN_FUNC_UNUSED,    /*  25 - DIO_27 */
    PIN_FUNC_UNUSED,    /*  26 - DIO_28 */
    PIN_FUNC_UNUSED,    /*  27 - DIO_29 */
    PIN_FUNC_UNUSED,    /*  28 - DIO_30 */
    PIN_FUNC_UNUSED,    /*  29 - DIO_00 */
    PIN_FUNC_UNUSED,    /*  30 - DIO_01 */
	
    PIN_FUNC_UNUSED,    /*  31 - DIO_17 */
    PIN_FUNC_UNUSED,    /*  32 - DIO_16 */
    PIN_FUNC_INVALID,   /*  33 - TCK */
    PIN_FUNC_INVALID,   /*  34 - TMS */
    PIN_FUNC_INVALID,   /*  35 - BPRST */
    PIN_FUNC_UNUSED,    /*  36 - DIO_18 */
    PIN_FUNC_UNUSED,    /*  37 - DIO_19 */
    PIN_FUNC_UNUSED,    /*  38 - DIO_20 */
    PIN_FUNC_UNUSED,    /*  39 - DIO_06 */
    PIN_FUNC_UNUSED,    /*  40 - DIO_07 */
};

/*
 * mapping of pins to an ADC channel
 */
const uint8_t digital_pin_to_adc_index[] = {
    NOT_ON_ADC,     /*  dummy */

    NOT_ON_ADC,     /*  1  - 3.3V */
    0,              /*  2  - DIO_23 */
    NOT_ON_ADC,     /*  3  - DIO_03 */
    NOT_ON_ADC,     /*  4  - DIO_02 */
    NOT_ON_ADC,     /*  5  - DIO_22 */
    1,              /*  6  - DIO_24 */
    NOT_ON_ADC,     /*  7  - DIO_10 */
    NOT_ON_ADC,     /*  8  - DIO_21 */
    NOT_ON_ADC,     /*  9  - DIO_04 */
    NOT_ON_ADC,     /*  10 - DIO_05 */
	
    NOT_ON_ADC,     /*  11 - DIO_15 */
    NOT_ON_ADC,     /*  12 - DIO_14 */
    NOT_ON_ADC,     /*  13 - DIO_13 */
    NOT_ON_ADC,     /*  14 - DIO_08 */
    NOT_ON_ADC,     /*  15 - DIO_09 */
    NOT_ON_ADC,     /*  16 - LPRST */
    NOT_ON_ADC,     /*  17 - NC */
    NOT_ON_ADC,     /*  18 - DIO_11 */
    NOT_ON_ADC,     /*  19 - DIO_12 */
    NOT_ON_ADC,     /*  20 - GND */
	
    NOT_ON_ADC,     /*  21 - 5V */
    NOT_ON_ADC,     /*  22 - GND */
    2,              /*  23 - DIO_25 */
    3,              /*  24 - DIO_26 */
    4,              /*  25 - DIO_27 */
    5,              /*  26 - DIO_28 */
    6,              /*  27 - DIO_29 */
    7,              /*  28 - DIO_30 */
    NOT_ON_ADC,     /*  29 - DIO_00 */
    NOT_ON_ADC,     /*  30 - DIO_01 */
	
    NOT_ON_ADC,     /*  31 - DIO_17 */
    NOT_ON_ADC,     /*  32 - DIO_16 */
    NOT_ON_ADC,     /*  33 - TCK */
    NOT_ON_ADC,     /*  34 - TMS */
    NOT_ON_ADC,     /*  35 - BPRST */
    NOT_ON_ADC,     /*  36 - DIO_18 */
    NOT_ON_ADC,     /*  37 - DIO_19 */
    NOT_ON_ADC,     /*  38 - DIO_20 */
    NOT_ON_ADC,     /*  39 - DIO_06 */
    NOT_ON_ADC,     /*  40 - DIO_07 */
};
