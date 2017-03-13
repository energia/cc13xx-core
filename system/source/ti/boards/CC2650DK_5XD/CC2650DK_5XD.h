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
/** ===========================================================================
 *  @file       CC2650DK_5XD.h
 *
 *  @brief      CC2650DK_5XD Board Specific header file.
 *
 *  The CC2650DK_5XD header file should be included in an application as
 *  follows:
 *  @code
 *  #include "CC2650DK_5XD.h"
 *  @endcode
 *
 *  ===========================================================================
 */
#ifndef __CC2650DK_5XD_BOARD_H__
#define __CC2650DK_5XD_BOARD_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes */
#include <ti/drivers/PIN.h>
#include <ti/devices/cc26x0/driverlib/ioc.h>

/* Externs */
extern const PIN_Config BoardGpioInitTable[];

/* Defines */
#define CC2650DK_5XD

/* Mapping of pins to board signals using general board aliases
 *      <board signal alias>         <pin mapping>
 */

/* Button Board */
#define CC2650DK_5XD_KEY_SELECT      IOID_9          /* P1.14 */
#define CC2650DK_5XD_KEY_UP          IOID_6          /* P1.10 */
#define CC2650DK_5XD_KEY_DOWN        IOID_4          /* P1.12 */
#define CC2650DK_5XD_KEY_LEFT        IOID_5          /* P1.6 */
#define CC2650DK_5XD_KEY_RIGHT       IOID_13         /* P1.8 */

/* GPIO */
#define CC2650DK_5XD_GPIO_LED_ON     1
#define CC2650DK_5XD_GPIO_LED_OFF    0

/* LEDs*/
#define CC2650DK_5XD_PIN_LED1        PIN_UNASSIGNED
#define CC2650DK_5XD_PIN_LED2        PIN_UNASSIGNED
#define CC2650DK_5XD_PIN_LED3        IOID_2          /* P1.2 */
#define CC2650DK_5XD_PIN_LED4        IOID_3          /* P1.4 */

/* LCD  Board */
#define CC2650DK_5XD_LCD_MODE        IOID_7          /* P1.11 */
#define CC2650DK_5XD_LCD_RST         PIN_UNASSIGNED
#define CC2650DK_5XD_LCD_CSN         IOID_8          /* P1.17 */

/* Power Board */
#define CC2650DK_5XD_3V3_EN          IOID_14         /* P1.15 */

/* PWM Outputs */
#define CC2650DK_5XD_PWMPIN0         CC2650DK_5XD_PIN_LED3
#define CC2650DK_5XD_PWMPIN1         CC2650DK_5XD_PIN_LED4
#define CC2650DK_5XD_PWMPIN2         PIN_UNASSIGNED
#define CC2650DK_5XD_PWMPIN3         PIN_UNASSIGNED
#define CC2650DK_5XD_PWMPIN4         PIN_UNASSIGNED
#define CC2650DK_5XD_PWMPIN5         PIN_UNASSIGNED
#define CC2650DK_5XD_PWMPIN6         PIN_UNASSIGNED
#define CC2650DK_5XD_PWMPIN7         PIN_UNASSIGNED

/* SPI Board */
#define CC2650DK_5XD_SPI0_MISO       IOID_12         /* P1.20 */
#define CC2650DK_5XD_SPI0_MOSI       IOID_11         /* P1.18 */
#define CC2650DK_5XD_SPI0_CLK        IOID_10         /* P1.16 */
#define CC2650DK_5XD_SPI0_CSN        PIN_UNASSIGNED

/* UART Board */
#define CC2650DK_5XD_UART_RX         IOID_1          /* P1.7 */
#define CC2650DK_5XD_UART_TX         IOID_0          /* P1.9 */
#define CC2650DK_5XD_UART_CTS        PIN_UNASSIGNED
#define CC2650DK_5XD_UART_RTS        PIN_UNASSIGNED

/*!
 *  @brief  Initialize the general board specific settings
 *
 *  This function initializes the general board specific settings.
 */
void CC2650DK_5XD_initGeneral(void);

/*!
 *  @def    CC2650DK_5XD_CryptoName
 *  @brief  Enum of Crypto names
 */
typedef enum CC2650DK_5XD_CryptoName {
    CC2650DK_5XD_CRYPTO0 = 0,
    CC2650DK_5XD_CRYPTOCOUNT
} CC2650DK_5XD_CryptoName;

/*!
 *  @def    CC2650DK_5XD_GPIOName
 *  @brief  Enum of GPIO names
 */
typedef enum CC2650DK_5XD_GPIOName {
    CC2650DK_5XD_GPIO_S1 = 0,
    CC2650DK_5XD_GPIO_S2,
    CC2650DK_5XD_GPIO_LED0,
    CC2650DK_5XD_GPIO_LED1,

    CC2650DK_5XD_GPIOCOUNT
} CC2650DK_5XD_GPIOName;

/*!
 *  @def    CC2650DK_5XD_GPTimerName
 *  @brief  Enum of GPTimer parts
 */
typedef enum CC2650DK_5XD_GPTimerName {
    CC2650DK_5XD_GPTIMER0A = 0,
    CC2650DK_5XD_GPTIMER0B,
    CC2650DK_5XD_GPTIMER1A,
    CC2650DK_5XD_GPTIMER1B,
    CC2650DK_5XD_GPTIMER2A,
    CC2650DK_5XD_GPTIMER2B,
    CC2650DK_5XD_GPTIMER3A,
    CC2650DK_5XD_GPTIMER3B,

    CC2650DK_5XD_GPTIMERPARTSCOUNT
} CC2650DK_5XD_GPTimerName;

/*!
 *  @def    CC2650DK_5XD_GPTimers
 *  @brief  Enum of GPTimers
 */
typedef enum CC2650DK_5XD_GPTimers {
    CC2650DK_5XD_GPTIMER0 = 0,
    CC2650DK_5XD_GPTIMER1,
    CC2650DK_5XD_GPTIMER2,
    CC2650DK_5XD_GPTIMER3,

    CC2650DK_5XD_GPTIMERCOUNT
} CC2650DK_5XD_GPTimers;

/*!
 *  @def    CC2650DK_5XD_PWM
 *  @brief  Enum of PWM outputs
 */
typedef enum CC2650DK_5XD_PWMName {
    CC2650DK_5XD_PWM0 = 0,
    CC2650DK_5XD_PWM1,
    CC2650DK_5XD_PWM2,
    CC2650DK_5XD_PWM3,
    CC2650DK_5XD_PWM4,
    CC2650DK_5XD_PWM5,
    CC2650DK_5XD_PWM6,
    CC2650DK_5XD_PWM7,

    CC2650DK_5XD_PWMCOUNT
} CC2650DK_5XD_PWMName;

/*!
 *  @def    CC2650DK_5XD_SPIName
 *  @brief  Enum of SPI names
 */
typedef enum CC2650DK_5XD_SPIName {
    CC2650DK_5XD_SPI0 = 0,

    CC2650DK_5XD_SPICOUNT
} CC2650DK_5XD_SPIName;

/*!
 *  @def    CC2650DK_5XD_UARTName
 *  @brief  Enum of UARTs
 */
typedef enum CC2650DK_5XD_UARTName {
    CC2650DK_5XD_UART0 = 0,

    CC2650DK_5XD_UARTCOUNT
} CC2650DK_5XD_UARTName;

/*!
 *  @def    CC2650DK_5XD_UDMAName
 *  @brief  Enum of DMA buffers
 */
typedef enum CC2650DK_5XD_UDMAName {
    CC2650DK_5XD_UDMA0 = 0,

    CC2650DK_5XD_UDMACOUNT
} CC2650DK_5XD_UDMAName;

/*!
 *  @def    CC2650DK_5XD_WatchdogName
 *  @brief  Enum of Watchdogs
 */
typedef enum CC2650DK_5XD_WatchdogName {
    CC2650DK_5XD_WATCHDOG0 = 0,

    CC2650DK_5XD_WATCHDOGCOUNT
} CC2650DK_5XD_WatchdogName;

#ifdef __cplusplus
}
#endif

#endif /* __CC2650DK_5XD_BOARD_H__ */
