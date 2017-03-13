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
/** ============================================================================
 *  @file       Board.h
 *
 *  @brief      CC1350 LaunchPad Board Specific header file.
 *
 *  The CC1350 header file should be included in an application as follows:
 *  @code
 *  #include <Board.h>
 *  @endcode
 *
 *  ============================================================================
 */
#ifndef __BOARD_H__
#define __BOARD_H__

#include <ti/drivers/UART.h>
#include <ti/drivers/I2C.h>
#include <ti/drivers/SPI.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __TI_COMPILER_VERSION__
#undef gcc
#define gcc 1
#endif

/** ============================================================================
 *  Includes
 *  ==========================================================================*/
#include <ti/drivers/PIN.h>
#include <driverlib/ioc.h>

/** ============================================================================
 *  Externs
 *  ==========================================================================*/
extern const PIN_Config BoardGpioInitTable[];

/** ============================================================================
 *  Defines
 *  ==========================================================================*/

/* Symbol by generic Board.c to include the correct PCB  specific Board.c */

/* Same RF Configuration as 7x7 EM */
#define CC1350EM_7ID

/* Mapping of pins to board signals using general board aliases
 *      <board signal alias>                <pin mapping>
 */

/* Discrete outputs */
#define Board_LED1                  IOID_6
#define Board_LED2                  IOID_7
#define Board_RLED                  IOID_6
#define Board_GLED                  IOID_7
#define Board_LED_ON                1
#define Board_LED_OFF               0

/* Discrete inputs */
#define Board_BTN1                  IOID_13
#define Board_BTN2                  IOID_14

/* I2C */
#define Board_I2C0_SDA0             IOID_5
#define Board_I2C0_SCL0             IOID_4

/* SPI */
#define Board_SPI0_MISO             IOID_8
#define Board_SPI0_MOSI             IOID_9
#define Board_SPI0_CLK              IOID_10
#define Board_SPI0_CSN              PIN_UNASSIGNED

#define Board_SPI1_MISO             PIN_UNASSIGNED
#define Board_SPI1_MOSI             PIN_UNASSIGNED
#define Board_SPI1_CLK              PIN_UNASSIGNED
#define Board_SPI1_CSN              PIN_UNASSIGNED

/* Generic GPTimer instance identifiers */
#define Board_GPTIMER0A             CC1350_LAUNCHXL_GPTIMER0A
#define Board_GPTIMER0B             CC1350_LAUNCHXL_GPTIMER0B
#define Board_GPTIMER1A             CC1350_LAUNCHXL_GPTIMER1A
#define Board_GPTIMER1B             CC1350_LAUNCHXL_GPTIMER1B
#define Board_GPTIMER2A             CC1350_LAUNCHXL_GPTIMER2A
#define Board_GPTIMER2B             CC1350_LAUNCHXL_GPTIMER2B
#define Board_GPTIMER3A             CC1350_LAUNCHXL_GPTIMER3A
#define Board_GPTIMER3B             CC1350_LAUNCHXL_GPTIMER3B

/* Generic PWM instance identifiers */
#define Board_PWM0                  CC1350_LAUNCHXL_PWM0
#define Board_PWM1                  CC1350_LAUNCHXL_PWM1
#define Board_PWM2                  CC1350_LAUNCHXL_PWM2
#define Board_PWM3                  CC1350_LAUNCHXL_PWM3
#define Board_PWM4                  CC1350_LAUNCHXL_PWM4
#define Board_PWM5                  CC1350_LAUNCHXL_PWM5
#define Board_PWM6                  CC1350_LAUNCHXL_PWM6
#define Board_PWM7                  CC1350_LAUNCHXL_PWM7

/* PWM outputs */
#define Board_PWMPIN0                       PIN_UNASSIGNED
#define Board_PWMPIN1                       PIN_UNASSIGNED
#define Board_PWMPIN2                       PIN_UNASSIGNED
#define Board_PWMPIN3                       PIN_UNASSIGNED
#define Board_PWMPIN4                       PIN_UNASSIGNED
#define Board_PWMPIN5                       PIN_UNASSIGNED
#define Board_PWMPIN6                       PIN_UNASSIGNED
#define Board_PWMPIN7                       PIN_UNASSIGNED

/* Analog Capable DIOs */
#define CC1350_LAUNCHXL_DIO23_ANALOG          IOID_23
#define CC1350_LAUNCHXL_DIO24_ANALOG          IOID_24
#define CC1350_LAUNCHXL_DIO25_ANALOG          IOID_25
#define CC1350_LAUNCHXL_DIO26_ANALOG          IOID_26
#define CC1350_LAUNCHXL_DIO27_ANALOG          IOID_27
#define CC1350_LAUNCHXL_DIO28_ANALOG          IOID_28
#define CC1350_LAUNCHXL_DIO29_ANALOG          IOID_29
#define CC1350_LAUNCHXL_DIO30_ANALOG          IOID_30

/* UART pins used by driver */
#define Board_UART_TX               IOID_3
#define Board_UART_RX               IOID_2

/** ============================================================================
 *  Instance identifiers
 *  ==========================================================================*/
/* Generic I2C instance identifiers */
#define Board_I2C                   CC1350_LAUNCHXL_I2C0
/* Generic SPI instance identifiers */
#define Board_SPI0                  CC1350_LAUNCHXL_SPI0
/* Generic UART instance identifiers */
#define Board_UART                  CC1350_LAUNCHXL_UART0
/* Generic PWM instance identifiers */
#define Board_PWM                   CC1350_LAUNCHXL_PWM0


/** ============================================================================
 *  Number of peripherals and their names
 *  ==========================================================================*/

/*!
 *  @def    CC1350_LAUNCHXL_ADCName
 *  @brief  Enum of ADCs
 */
typedef enum CC1350_LAUNCHXL_ADCName {
    CC1350_LAUNCHXL_ADC0 = 0,
    CC1350_LAUNCHXL_ADC1,
    CC1350_LAUNCHXL_ADC2,
    CC1350_LAUNCHXL_ADC3,
    CC1350_LAUNCHXL_ADC4,
    CC1350_LAUNCHXL_ADC5,
    CC1350_LAUNCHXL_ADC6,
    CC1350_LAUNCHXL_ADC7,
    CC1350_LAUNCHXL_ADCDCOUPL,
    CC1350_LAUNCHXL_ADCVSS,
    CC1350_LAUNCHXL_ADCVDDS,

    CC1350_LAUNCHXL_ADCCOUNT
} CC1350_LAUNCHXL_ADCName;

/*!
 *  @def    CC1350_I2CName
 *  @brief  Enum of I2C names on the CC1350 dev board
 */
typedef enum CC1350_LAUNCHXL_I2CName {
    CC1350_LAUNCHXL_I2C0 = 0,
    CC1350_LAUNCHXL_I2CCOUNT
} CC1350_LAUNCHXL_I2CName;

/*!
 *  @def    CC1350_CryptoName
 *  @brief  Enum of Crypto names on the CC1350 dev board
 */
typedef enum CC1350_LAUNCHXL_CryptoName {
    CC1350_LAUNCHXL_CRYPTO0 = 0,
    CC1350_LAUNCHXL_CRYPTOCOUNT
} CC1350_LAUNCHXL_CryptoName;


/*!
 *  @def    CC1350_SPIName
 *  @brief  Enum of SPI names on the CC1350 dev board
 */
typedef enum CC1350_LAUNCHXL_SPIName {
    CC1350_LAUNCHXL_SPI0 = 0,
    CC1350_LAUNCHXL_SPICOUNT
} CC1350_LAUNCHXL_SPIName;

/*!
 *  @def    CC1350_UARTName
 *  @brief  Enum of UARTs on the CC1350 dev board
 */
typedef enum CC1350_LAUNCHXL_UARTName {
    CC1350_LAUNCHXL_UART0 = 0,
    CC1350_LAUNCHXL_UARTCOUNT
} CC1350_LAUNCHXL_UARTName;

/*!
 *  @def    CC1350_UdmaName
 *  @brief  Enum of DMA buffers
 */
typedef enum CC1350_LAUNCHXL_UdmaName {
    CC1350_LAUNCHXL_UDMA0 = 0,
    CC1350_LAUNCHXL_UDMACOUNT
} CC1350_LAUNCHXL_UdmaName;

/*!
 *  @def    CC1350_LAUNCHXL_GPTimerName
 *  @brief  Enum of GPTimer parts
 */
typedef enum CC1350_LAUNCHXL_GPTimerName
{
    CC1350_LAUNCHXL_GPTIMER0A = 0,
    CC1350_LAUNCHXL_GPTIMER0B,
    CC1350_LAUNCHXL_GPTIMER1A,
    CC1350_LAUNCHXL_GPTIMER1B,
    CC1350_LAUNCHXL_GPTIMER2A,
    CC1350_LAUNCHXL_GPTIMER2B,
    CC1350_LAUNCHXL_GPTIMER3A,
    CC1350_LAUNCHXL_GPTIMER3B,
    CC1350_LAUNCHXL_GPTIMERPARTSCOUNT
} CC1350_LAUNCHXL_GPTimerName;

/*!
 *  @def    CC1350_LAUNCHXL_GPTimers
 *  @brief  Enum of GPTimers
 */
typedef enum CC1350_LAUNCHXL_GPTimers
{
    CC1350_LAUNCHXL_GPTIMER0 = 0,
    CC1350_LAUNCHXL_GPTIMER1,
    CC1350_LAUNCHXL_GPTIMER2,
    CC1350_LAUNCHXL_GPTIMER3,
    CC1350_LAUNCHXL_GPTIMERCOUNT
} CC1350_LAUNCHXL_GPTimers;

/*!
 *  @def    CC1350_PWMName
 *  @brief  Enum of PWM pin names on the CC1350 dev board
 */
typedef enum CC1350_LAUNCHXL_PWMName {
    CC1350_LAUNCHXL_PWM0 = 0, /* PWM output from TIMERA0 side A */
    CC1350_LAUNCHXL_PWM1 = 1, /* PWM output from TIMERA0 side B */
    CC1350_LAUNCHXL_PWM2 = 2, /* PWM output from TIMERA1 side A */
    CC1350_LAUNCHXL_PWM3 = 3, /* PWM output from TIMERA1 side B */
    CC1350_LAUNCHXL_PWM4 = 4, /* PWM output from TIMERA2 side A */
    CC1350_LAUNCHXL_PWM5 = 5, /* PWM output from TIMERA2 side B */
    CC1350_LAUNCHXL_PWM6 = 6, /* PWM output from TIMERA3 side A */
    CC1350_LAUNCHXL_PWM7 = 7, /* PWM output from TIMERA3 side B */
    CC1350_LAUNCHXL_PWMCOUNT
} CC1350_LAUNCHXL_PWMName;

#ifdef __cplusplus
}
#endif

/* These #defines allow us to reuse TI-RTOS across other device families */
#define     Board_LED0              Board_LED1

#define     Board_BUTTON0           Board_BTN1
#define     Board_BUTTON1           Board_BTN2

#define     Board_I2C0              Board_I2C
#define     Board_UART0             Board_UART
#define     Board_WATCHDOG0         Board_WATCHDOG

#define     Board_initGeneral()     PIN_init(BoardGpioInitTable)
#define     Board_initWatchdog()    Watchdog_init()

#endif /* __BOARD_H__ */
