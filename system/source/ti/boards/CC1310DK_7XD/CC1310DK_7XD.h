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
/** ===========================================================================
 *  @file       CC1310DK_7XD.h
 *
 *  @brief      CC1310DK_7XD Board Specific header file.
 *
 *  The CC1310DK_7XD header file should be included in an application as
 *  follows:
 *  @code
 *  #include "CC1310DK_7XD.h"
 *  @endcode
 *
 *  ===========================================================================
 */
#ifndef __CC1310DK_7XD_BOARD_H__
#define __CC1310DK_7XD_BOARD_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes */
#include <ti/drivers/PIN.h>
#include <ti/devices/cc13x0/driverlib/ioc.h>

/* Externs */
extern const PIN_Config BoardGpioInitTable[];

/* Defines */
#define CC1310DK_7XD

/* Mapping of pins to board signals using general board aliases
 *      <board signal alias>         <pin mapping>   <comments>
 */

/* Accelerometer */
#define CC1310DK_7XD_ACC_PWR         IOID_20         /* P2.8 */
#define CC1310DK_7XD_ACC_CSN         IOID_24         /* P2.10 */

/* Ambient Light Sensor */
#define CC1310DK_7XD_ALS_OUT         IOID_23         /* P2.5 */
#define CC1310DK_7XD_ALS_PWR         IOID_26         /* P2.6 */

/* Analog Capable DIOs */
#define CC1310DK_7XD_DIO23_ANALOG    IOID_23
#define CC1310DK_7XD_DIO24_ANALOG    IOID_24
#define CC1310DK_7XD_DIO25_ANALOG    IOID_25
#define CC1310DK_7XD_DIO26_ANALOG    IOID_26
#define CC1310DK_7XD_DIO27_ANALOG    IOID_27
#define CC1310DK_7XD_DIO28_ANALOG    IOID_28
#define CC1310DK_7XD_DIO29_ANALOG    IOID_29
#define CC1310DK_7XD_DIO30_ANALOG    IOID_30

/* Button Board */
#define CC1310DK_7XD_KEY_SELECT      IOID_11         /* P1.14 */
#define CC1310DK_7XD_KEY_UP          IOID_19         /* P1.10 */
#define CC1310DK_7XD_KEY_DOWN        IOID_12         /* P1.12 */
#define CC1310DK_7XD_KEY_LEFT        IOID_15         /* P1.6  */
#define CC1310DK_7XD_KEY_RIGHT       IOID_18         /* P1.8  */

/* GPIO */
#define CC1310DK_7XD_GPIO_LED_ON     1
#define CC1310DK_7XD_GPIO_LED_OFF    0

/* LCD  Board */
#define CC1310DK_7XD_LCD_MODE        IOID_4          /* P1.11 */
#define CC1310DK_7XD_LCD_RST         IOID_5          /* P1.13 */
#define CC1310DK_7XD_LCD_CSN         IOID_14         /* P1.17 */

/* LEDs */
#define CC1310DK_7XD_LED_ON          1
#define CC1310DK_7XD_LED_OFF         0
#define CC1310DK_7XD_PIN_LED1        IOID_25         /* P2.11 */
#define CC1310DK_7XD_PIN_LED2        IOID_27         /* P2.13 */
#define CC1310DK_7XD_PIN_LED3        IOID_7          /* P1.2  */
#define CC1310DK_7XD_PIN_LED4        IOID_6          /* P1.4  */

/* Power Board */
#define CC1310DK_7XD_3V3_EN          IOID_13         /* P1.15 */

/* PWM Outputs */
#define CC1310DK_7XD_PWMPIN0         CC1310DK_7XD_PIN_LED1
#define CC1310DK_7XD_PWMPIN1         CC1310DK_7XD_PIN_LED2
#define CC1310DK_7XD_PWMPIN2         PIN_UNASSIGNED
#define CC1310DK_7XD_PWMPIN3         PIN_UNASSIGNED
#define CC1310DK_7XD_PWMPIN4         PIN_UNASSIGNED
#define CC1310DK_7XD_PWMPIN5         PIN_UNASSIGNED
#define CC1310DK_7XD_PWMPIN6         PIN_UNASSIGNED
#define CC1310DK_7XD_PWMPIN7         PIN_UNASSIGNED

/* SD Card */
#define CC1310DK_7XD_SDCARD_CSN      IOID_30         /* P2.12 */

/* SPI Board */
#define CC1310DK_7XD_SPI0_MISO       IOID_8          /* P1.20 */
#define CC1310DK_7XD_SPI0_MOSI       IOID_9          /* P1.18 */
#define CC1310DK_7XD_SPI0_CLK        IOID_10         /* P1.16 */
#define CC1310DK_7XD_SPI0_CSN        PIN_UNASSIGNED  /* P1.14, separate CSn for LCD, SDCARD & ACC */
#define CC1310DK_7XD_SPI1_MISO       IOID_24         /* RF2.10 for testing only */
#define CC1310DK_7XD_SPI1_MOSI       IOID_23         /* RF2.5  for testing only */
#define CC1310DK_7XD_SPI1_CLK        IOID_30         /* RF2.12 for testing only */
#define CC1310DK_7XD_SPI1_CSN        PIN_UNASSIGNED  /* RF2.6  for testing only */

/* UART Board */
#define CC1310DK_7XD_UART_RX         IOID_2          /* P1.7  */
#define CC1310DK_7XD_UART_TX         IOID_3          /* P1.9  */
#define CC1310DK_7XD_UART_CTS        IOID_22         /* P1.3  */
#define CC1310DK_7XD_UART_RTS        IOID_21         /* P2.18 */

/*!
 *  @brief  Initialize the general board specific settings
 *
 *  This function initializes the general board specific settings.
 */
void CC1310DK_7XD_initGeneral(void);

/*!
 *  @def    CC1310DK_7XD_ADCBufName
 *  @brief  Enum of ADCBufs
 */
typedef enum CC1310DK_7XD_ADCBufName {
    CC1310DK_7XD_ADCBUF0 = 0,

    CC1310DK_7XD_ADCBUFCOUNT
} CC1310DK_7XD_ADCBufName;

/*!
 *  @def    CC1310_LAUNCHXL_ADCBuf0ChannelName
 *  @brief  Enum of ADCBuf channels
 */
typedef enum  CC1310DK_7XD_ADCBuf0ChannelName {
     CC1310DK_7XD_ADCBUF0CHANNEL0 = 0,
     CC1310DK_7XD_ADCBUF0CHANNELVDDS,
     CC1310DK_7XD_ADCBUF0CHANNELDCOUPL,
     CC1310DK_7XD_ADCBUF0CHANNELVSS,

     CC1310DK_7XD_ADCBUF0CHANNELCOUNT
}  CC1310DK_7XD_ADCBuf0ChannelName;

/*!
 *  @def    CC1310DK_7XD_ADCName
 *  @brief  Enum of ADCs
 */
typedef enum CC1310DK_7XD_ADCName {
    CC1310DK_7XD_ADCALS = 0,
    CC1310DK_7XD_ADCDCOUPL,
    CC1310DK_7XD_ADCVSS,
    CC1310DK_7XD_ADCVDDS,

    CC1310DK_7XD_ADCCOUNT
} CC1310DK_7XD_ADCName;

/*!
 *  @def    CC1310DK_7XD_CryptoName
 *  @brief  Enum of Crypto names
 */
typedef enum CC1310DK_7XD_CryptoName {
    CC1310DK_7XD_CRYPTO0 = 0,

    CC1310DK_7XD_CRYPTOCOUNT
} CC1310DK_7XD_CryptoName;

/*!
 *  @def    CC1310DK_7XD_GPIOName
 *  @brief  Enum of GPIO names
 */
typedef enum CC1310DK_7XD_GPIOName {
    CC1310DK_7XD_GPIO_S1 = 0,
    CC1310DK_7XD_GPIO_S2,
    CC1310DK_7XD_GPIO_LED1,
    CC1310DK_7XD_GPIO_LED2,
    CC1310DK_7XD_GPIO_LED3,
    CC1310DK_7XD_GPIO_LED4,

    CC1310DK_7XD_GPIOCOUNT
} CC1310DK_7XD_GPIOName;

/*!
 *  @def    CC1310DK_7XD_GPTimerName
 *  @brief  Enum of GPTimer parts
 */
typedef enum CC1310DK_7XD_GPTimerName {
    CC1310DK_7XD_GPTIMER0A = 0,
    CC1310DK_7XD_GPTIMER0B,
    CC1310DK_7XD_GPTIMER1A,
    CC1310DK_7XD_GPTIMER1B,
    CC1310DK_7XD_GPTIMER2A,
    CC1310DK_7XD_GPTIMER2B,
    CC1310DK_7XD_GPTIMER3A,
    CC1310DK_7XD_GPTIMER3B,

    CC1310DK_7XD_GPTIMERPARTSCOUNT
} CC1310DK_7XD_GPTimerName;

/*!
 *  @def    CC1310DK_7XD_GPTimers
 *  @brief  Enum of GPTimers
 */
typedef enum CC1310DK_7XD_GPTimers {
    CC1310DK_7XD_GPTIMER0 = 0,
    CC1310DK_7XD_GPTIMER1,
    CC1310DK_7XD_GPTIMER2,
    CC1310DK_7XD_GPTIMER3,

    CC1310DK_7XD_GPTIMERCOUNT
} CC1310DK_7XD_GPTimers;

/*!
 *  @def    CC1310DK_7XD_PWM
 *  @brief  Enum of PWM outputs on the board
 */
typedef enum CC1310DK_7XD_PWMName {
    CC1310DK_7XD_PWM0 = 0,
    CC1310DK_7XD_PWM1,
    CC1310DK_7XD_PWM2,
    CC1310DK_7XD_PWM3,
    CC1310DK_7XD_PWM4,
    CC1310DK_7XD_PWM5,
    CC1310DK_7XD_PWM6,
    CC1310DK_7XD_PWM7,

    CC1310DK_7XD_PWMCOUNT
} CC1310DK_7XD_PWMName;

/*!
 *  @def    CC1310DK_7XD_SPIName
 *  @brief  Enum of SPI names
 */
typedef enum CC1310DK_7XD_SPIName {
    CC1310DK_7XD_SPI0 = 0,
    CC1310DK_7XD_SPI1,

    CC1310DK_7XD_SPICOUNT
} CC1310DK_7XD_SPIName;

/*!
 *  @def    CC1310DK_7XD_UARTName
 *  @brief  Enum of UARTs
 */
typedef enum CC1310DK_7XD_UARTName {
    CC1310DK_7XD_UART0 = 0,

    CC1310DK_7XD_UARTCOUNT
} CC1310DK_7XD_UARTName;

/*!
 *  @def    CC1310DK_7XD_UDMAName
 *  @brief  Enum of DMA buffers
 */
typedef enum CC1310DK_7XD_UDMAName {
    CC1310DK_7XD_UDMA0 = 0,

    CC1310DK_7XD_UDMACOUNT
} CC1310DK_7XD_UDMAName;

/*!
 *  @def    CC1310DK_7XD_WatchdogName
 *  @brief  Enum of Watchdogs
 */
typedef enum CC1310DK_7XD_WatchdogName {
    CC1310DK_7XD_WATCHDOG0 = 0,

    CC1310DK_7XD_WATCHDOGCOUNT
} CC1310DK_7XD_WatchdogName;

#ifdef __cplusplus
}
#endif

#endif /* __CC1310DK_7XD_BOARD_H__ */
