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

/*
 *  ====================== Board.c =============================================
 *  This file is responsible for setting up the board specific items for the
 *  CC1350 SensorTag.
 *
 *  NB! This board file is for PCB version 1.2
 */


/*
 *  ====================== Includes ============================================
 */
#include <inc/hw_memmap.h>
#include <inc/hw_ints.h>
#include <driverlib/ioc.h>
#include <driverlib/udma.h>
#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <ti/sysbios/family/arm/m3/Hwi.h>

#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26XX.h>
#include "Board.h"

/*
 *  ========================= IO driver initialization =========================
 *  From main, PIN_init(BoardGpioInitTable) should be called to setup safe
 *  settings for this board.
 *  When a pin is allocated and then de-allocated, it will revert to the state
 *  configured in this table
*/
#include <ti/drivers/PIN.h>
#include  <ti/drivers/pin/PINCC26XX.h>

#if defined(__TI_COMPILER_VERSION__)
#pragma DATA_SECTION(BoardGpioInitTable, ".const:BoardGpioInitTable")
#pragma DATA_SECTION(PINCC26XX_hwAttrs, ".const:PINCC26XX_hwAttrs")
#endif

const PIN_Config BoardGpioInitTable[] = {

    Board_STK_LED1   | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,     /* LED initially off             */
    Board_KEY_LEFT   | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_BOTHEDGES | PIN_HYSTERESIS,        /* Button is active low          */
    Board_KEY_RIGHT  | PIN_INPUT_EN | PIN_PULLUP | PIN_IRQ_BOTHEDGES | PIN_HYSTERESIS,        /* Button is active low          */
    Board_RELAY      | PIN_INPUT_EN | PIN_PULLDOWN | PIN_IRQ_BOTHEDGES | PIN_HYSTERESIS,      /* Relay is active high          */
    Board_MPU_INT    | PIN_INPUT_EN | PIN_PULLDOWN | PIN_IRQ_NEGEDGE | PIN_HYSTERESIS,        /* MPU_INT is active low         */
    Board_TMP_RDY    | PIN_INPUT_EN | PIN_PULLUP | PIN_HYSTERESIS,                            /* TMP_RDY is active high        */
    Board_BUZZER     | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,     /* Buzzer initially off          */
    Board_MPU_POWER  | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MAX,    /* MPU initially on              */
    Board_MIC_POWER  | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MIN,     /* MIC initially off             */
    Board_SPI_FLASH_CS | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MIN,  /* External flash chip select    */
    Board_SPI_DEVPK_CS | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MIN,   /* DevPack chip select           */
    Board_AUDIO_DI | PIN_INPUT_EN | PIN_PULLDOWN,                                             /* Audio DI                      */
    Board_AUDIODO | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_DRVSTR_MIN,       /* Audio data out                */
    Board_AUDIO_CLK | PIN_INPUT_EN | PIN_PULLDOWN,                                            /* DevPack */
    Board_DP2 | PIN_INPUT_EN | PIN_PULLDOWN,                                                  /* DevPack */
    Board_DP1 | PIN_INPUT_EN | PIN_PULLDOWN,                                                  /* DevPack */
    Board_DP0 | PIN_INPUT_EN | PIN_PULLDOWN,                                                  /* DevPack */
    Board_DP3 | PIN_INPUT_EN | PIN_PULLDOWN,                                                  /* DevPack */
    Board_UART_TX | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL,                        /* DevPack */
    Board_UART_RX | PIN_INPUT_EN | PIN_PULLDOWN,                                              /* DevPack */
    Board_DEVPK_ID | PIN_INPUT_EN | PIN_NOPULL,                                               /* Device pack ID - external PU  */
    Board_SPI0_MOSI | PIN_INPUT_EN | PIN_PULLDOWN,                                            /* SPI master out - slave in */
    Board_SPI0_MISO | PIN_INPUT_EN | PIN_PULLDOWN,                                            /* SPI master in - slave out */
    Board_SPI0_CLK | PIN_INPUT_EN | PIN_PULLDOWN,                                             /* SPI clock */

    PIN_TERMINATE
};

PINCC26XX_HWAttrs PINCC26XX_hwAttrs = {
    .intPriority = ~0,
    .swiPriority = 0
};

/*============================================================================*/

/*
 * ======== GPIO driver ========
 */

#include <ti/drivers/GPIO.h>
#include <ti/drivers/gpio/GPIOCC26XX.h>

/* GPIO configuration structure definitions */
/* Place into subsections to allow the TI linker to remove items properly */
#if defined(__TI_COMPILER_VERSION__)
#pragma DATA_SECTION(GPIOCC26XX_config, ".const:GPIOCC26XX_config")
#pragma DATA_SECTION(gpioPinConfigs, ".data:gpioPinConfigs")
#pragma DATA_SECTION(gpioCallbackFunctions, ".data:gpioCallbackFunctions")
#endif

GPIO_PinConfig gpioPinConfigs[] = {
    /* port_pin */
    GPIOCC26XX_EMPTY_PIN | GPIO_DO_NOT_CONFIG,  /*  0  - dummy */
                          
    /* pins 1-10 */
    GPIOCC26XX_EMPTY_PIN | GPIO_DO_NOT_CONFIG,  /*  1  - VDD */
    GPIOCC26XX_EMPTY_PIN | GPIO_DO_NOT_CONFIG,  /*  2  - GND */
    GPIOCC26XX_DIO_06 | GPIO_DO_NOT_CONFIG,     /*  3  - DIO_06 SCL */
    GPIOCC26XX_DIO_05 | GPIO_DO_NOT_CONFIG,     /*  4  - DIO_05 SDA */
    GPIOCC26XX_DIO_16 | GPIO_DO_NOT_CONFIG,     /*  5  - DIO_16 DP12/AUDIO FS/TD0 */
    GPIOCC26XX_DIO_03 | GPIO_DO_NOT_CONFIG,     /*  6  - DIO_03 DP7/AUDIO CLK */
    GPIOCC26XX_DIO_20 | GPIO_DO_NOT_CONFIG,     /*  7  - DIO_20 DP11/CSN */
    GPIOCC26XX_DIO_22 | GPIO_DO_NOT_CONFIG,     /*  8  - DIO_22 DP6/AUDIO DO */
    GPIOCC26XX_EMPTY_PIN | GPIO_DO_NOT_CONFIG,  /*  9  - VDD */
    GPIOCC26XX_DIO_29 | GPIO_DO_NOT_CONFIG,     /*  10 - DIO_29 DP5/UART_TX */
                          
    /* pins 11-20 */
    GPIOCC26XX_DIO_19 | GPIO_DO_NOT_CONFIG,     /*  11 - DIO_19 DP10/MOSI */
    GPIOCC26XX_DIO_28 | GPIO_DO_NOT_CONFIG,     /*  12 - DIO_28 DP4/UART_RX */
    GPIOCC26XX_DIO_18 | GPIO_DO_NOT_CONFIG,     /*  13 - DIO_18 DP9/MISO */
    GPIOCC26XX_DIO_27 | GPIO_DO_NOT_CONFIG,     /*  14 - DIO_27 DP3 */
    GPIOCC26XX_DIO_17 | GPIO_DO_NOT_CONFIG,     /*  15 - DIO_17 DP8/SCLK/TDI */
    GPIOCC26XX_DIO_23 | GPIO_DO_NOT_CONFIG,     /*  16 - DIO_23 DP2  */
    GPIOCC26XX_DIO_30 | GPIO_DO_NOT_CONFIG,     /*  17 - DIO_30 DP_ID */
    GPIOCC26XX_DIO_24 | GPIO_DO_NOT_CONFIG,     /*  18 - DIO_24 DP1 */
    GPIOCC26XX_EMPTY_PIN | GPIO_DO_NOT_CONFIG,  /*  19 - POWER_GOOD */
    GPIOCC26XX_DIO_25 | GPIO_DO_NOT_CONFIG,     /*  20 - DIO_25 DP0 */
                          
    /* virtual pins 21-30 */
    GPIOCC26XX_DIO_02 | GPIO_DO_NOT_CONFIG,     /*  21 - DIO_02 AUDIO DI */
    GPIOCC26XX_DIO_12 | GPIO_DO_NOT_CONFIG,     /*  22 - DIO_12 MPU PWR */
    GPIOCC26XX_DIO_21 | GPIO_DO_NOT_CONFIG,     /*  23 - DIO_21 BUZZER */
    GPIOCC26XX_DIO_01 | GPIO_DO_NOT_CONFIG,     /*  24 - DIO_01 REED */
    GPIOCC26XX_DIO_04 | GPIO_DO_NOT_CONFIG,     /*  25 - DIO_04 BUTTON1 */
    GPIOCC26XX_DIO_11 | GPIO_DO_NOT_CONFIG,     /*  26 - DIO_11 TMP RDY */
    GPIOCC26XX_DIO_10 | GPIO_DO_NOT_CONFIG,     /*  27 - DIO_10 LED1 */
    GPIOCC26XX_DIO_07 | GPIO_DO_NOT_CONFIG,     /*  28 - DIO_07 MPU INT */
    GPIOCC26XX_DIO_13 | GPIO_DO_NOT_CONFIG,     /*  29 - DIO_13 MIC PWR */
    GPIOCC26XX_DIO_15 | GPIO_DO_NOT_CONFIG,     /*  30 - DIO_15 BUTTON2 */
                          
    /* virtual pin 31 */
    GPIOCC26XX_EMPTY_PIN | GPIO_DO_NOT_CONFIG,  /*  31 - dummy (LED2 on CC2650 STK) */
    GPIOCC26XX_DIO_14 | GPIO_DO_NOT_CONFIG,     /*  32 - DIO_14 FLASH CS */
};

GPIO_CallbackFxn gpioCallbackFunctions[] = {
    /* port_pin */
    NULL,  /*  0  - dummy */

    /* pins 1-10 */
    NULL,  /*  1  - VDD */
    NULL,  /*  2  - GND */
    NULL,  /*  3  - DIO_06 SCL */
    NULL,  /*  4  - DIO_05 SDA */
    NULL,  /*  5  - DIO_16 DP12/AUDIO FS/TD0 */
    NULL,  /*  6  - DIO_03 DP7/AUDIO CLK */
    NULL,  /*  7  - DIO_20 DP11/CSN */
    NULL,  /*  8  - DIO_22 DP6/AUDIO DO */
    NULL,  /*  9  - VDD */
    NULL,  /*  10 - DIO_29 DP5/UART_TX */
                    
    /* pins 11-20 */
    NULL,  /*  11 - DIO_19 DP10/MOSI */
    NULL,  /*  12 - DIO_28 DP4/UART_RX */
    NULL,  /*  13 - DIO_18 DP9/MISO */
    NULL,  /*  14 - DIO_27 DP3 */
    NULL,  /*  15 - DIO_17 DP8/SCLK/TDI */
    NULL,  /*  16 - DIO_23 DP2  */
    NULL,  /*  17 - DIO_30 DP_ID */
    NULL,  /*  18 - DIO_24 DP1 */
    NULL,  /*  19 - POWER_GOOD */
    NULL,  /*  20 - DIO_25 DP0 */
                    
    /* pins 21-30 */
    NULL,  /*  21 - DIO_02 AUDIO DI */
    NULL,  /*  22 - DIO_12 MPU PWR */
    NULL,  /*  23 - DIO_21 BUZZER */
    NULL,  /*  24 - DIO_01 REED */
    NULL,  /*  25 - DIO_04 BUTTON1 */
    NULL,  /*  26 - DIO_11 TMP RDY */
    NULL,  /*  27 - DIO_10 LED1 */
    NULL,  /*  28 - DIO_07 MPU INT */
    NULL,  /*  29 - DIO_13 MIC PWR */
    NULL,  /*  30 - DIO_15 BUTTON2 */
                    
    /* pin 31 */
    NULL,  /*  31 - Dummy (LED2 on CC2650 STK) */
    NULL,  /*  32 - DIO_14 FLASH CS */
};

/* User requested callback functions for the GPIO input signals */

/* The device-specific GPIO_config structure */
const GPIOCC26XX_Config GPIOCC26XX_config = {
    .pinConfigs = (GPIO_PinConfig *)gpioPinConfigs,
    .callbacks = (GPIO_CallbackFxn *)gpioCallbackFunctions,
    .numberOfPinConfigs = sizeof(gpioPinConfigs)/sizeof(GPIO_PinConfig),
    .numberOfCallbacks = sizeof(gpioCallbackFunctions)/sizeof(GPIO_CallbackFxn),
    .intPriority = ~0
};

/*
 *  ======== Board_initGPIO ========
 */
void Board_initGPIO(void)
{
    /* set up initial GPIO pin configurations */
    GPIO_init();
}

/*
 *  ============================= UART begin ===================================
*/
#if defined(__TI_COMPILER_VERSION__)
#pragma DATA_SECTION(UART_config, ".const:UART_config")
#pragma DATA_SECTION(uartCC26XXHWAttrs, ".const:uartCC26XXHWAttrs")
#endif

/* Include drivers */
#include <ti/drivers/UART.h>
#include <ti/drivers/uart/UARTCC26XX.h>

/* UART objects */
UARTCC26XX_Object uartCC26XXObjects[CC1350_UARTCOUNT];

/* UART hardware parameter structure, also used to assign UART pins */
const UARTCC26XX_HWAttrsV1 uartCC26XXHWAttrs[CC1350_UARTCOUNT] = {
    {   /* CC1350_UART0 */
        .baseAddr = UART0_BASE,
        .powerMngrId = PowerCC26XX_PERIPH_UART0,
        .intNum = INT_UART0_COMB,    /* <driverlib>/inc/hw_ints.h */
        .intPriority = (~0),
        .swiPriority = 0,
        .txPin = Board_DP5_UARTTX,
        .rxPin = Board_DP4_UARTRX,
        .ctsPin = PIN_UNASSIGNED,
        .rtsPin = PIN_UNASSIGNED
    },
};

/* UART configuration structure */
const UART_Config UART_config[] = {
    {
        .fxnTablePtr = &UARTCC26XX_fxnTable,
        .object      = &uartCC26XXObjects[0],
        .hwAttrs     = &uartCC26XXHWAttrs[0]
    },
    {NULL, NULL, NULL}
};

/*
 *  ======== Board_openUART ========
 *  Initialize the UART driver.
 *  Initialize the UART port's pins.
 *  Open the UART port.
 */
UART_Handle Board_openUART(UInt uartPortIndex, UART_Params *uartParams)
{
    /* Initialize the UART driver */
    /* By design, UART_init() is idempotent */
    UART_init();

    /* initialize the pins associated with the respective UART */
    switch(uartPortIndex) {
        case 0:
            break;

        default:
            return (NULL);
    }

    /* open the UART */
    return (UART_open(uartPortIndex, uartParams));
}

/*
 *  ============================= UART end =====================================
*/

/*
 *  ============================= UDMA begin ===================================
*/
/* Place into subsections to allow the TI linker to remove items properly */
#if defined(__TI_COMPILER_VERSION__)
#pragma DATA_SECTION(UDMACC26XX_config, ".const:UDMACC26XX_config")
#pragma DATA_SECTION(udmaHWAttrs, ".const:udmaHWAttrs")
#endif

/* Include drivers */
#include <ti/drivers/dma/UDMACC26XX.h>

/* UDMA objects */
UDMACC26XX_Object udmaObjects[CC1350_UDMACOUNT];

/* UDMA configuration structure */
const UDMACC26XX_HWAttrs udmaHWAttrs[CC1350_UDMACOUNT] = {
    {
        .baseAddr = UDMA0_BASE, 
        .powerMngrId = PowerCC26XX_PERIPH_UDMA,
        .intNum = INT_DMA_ERR,    /* <driverlib>/inc/hw_ints.h */
        .intPriority = (~0)
    },
};

/* UDMA configuration structure */
const UDMACC26XX_Config UDMACC26XX_config[] = {
    {
        .object  = &udmaObjects[0],
        .hwAttrs = &udmaHWAttrs[0]
    },
    {NULL, NULL}
};
/*
 *  ============================= UDMA end =====================================
*/

/*
 *  ========================== SPI DMA begin ===================================
*/
/* Place into subsections to allow the TI linker to remove items properly */
#if defined(__TI_COMPILER_VERSION__)
#pragma DATA_SECTION(SPI_config, ".const:SPI_config")
#pragma DATA_SECTION(spiCC26XXDMAHWAttrs, ".const:spiCC26XXDMAHWAttrs")
#endif

/* Include drivers */
#include <ti/drivers/spi/SPICC26XXDMA.h>

/* SPI objects */
SPICC26XXDMA_Object spiCC26XXDMAObjects[CC1350_SPICOUNT];

/* SPI configuration structure, describing which pins are to be used */
const SPICC26XXDMA_HWAttrsV1 spiCC26XXDMAHWAttrs[CC1350_SPICOUNT] = {
    {   /* SENSORTAG_CC1350_SPI0 */
        .baseAddr = SSI0_BASE,
        .intNum = INT_SSI0_COMB,   /* <driverlib>/inc/hw_ints.h */
        .intPriority = 0xC0,       /* make SPI interrupt one priority higher than default */
        .defaultTxBufValue = 0,
        .powerMngrId = PowerCC26XX_PERIPH_SSI0,
        .rxChannelBitMask = 1<<UDMA_CHAN_SSI0_RX,
        .txChannelBitMask = 1<<UDMA_CHAN_SSI0_TX,
        .mosiPin = Board_SPI0_MOSI,
        .misoPin = Board_SPI0_MISO,
        .clkPin = Board_SPI0_CLK,
        .csnPin = PIN_UNASSIGNED     /* External flash / DevPk uses SPI0 */
    },
    {   /* SENSORTAG_CC1350_SPI1 */
        .baseAddr = SSI1_BASE,
        .intNum = INT_SSI1_COMB,     /* <driverlib>/inc/hw_ints.h */
        .intPriority = ~0,
        .defaultTxBufValue = 0,
        .powerMngrId = PowerCC26XX_PERIPH_SSI1,
        .rxChannelBitMask  = 1<<UDMA_CHAN_SSI1_RX,
        .txChannelBitMask  = 1<<UDMA_CHAN_SSI1_TX,
        .mosiPin = Board_SPI1_MOSI,
        .misoPin = Board_SPI1_MISO,
        .clkPin = Board_SPI1_CLK,
        .csnPin = Board_SPI1_CSN
    }
};

/* SPI configuration structure */
const SPI_Config SPI_config[] = {
    /* SENSORTAG_CC1350_SPI0 */
    {&SPICC26XXDMA_fxnTable, &spiCC26XXDMAObjects[0], &spiCC26XXDMAHWAttrs[0]},
    /* SENSORTAG_CC1350_SPI1 */
    {&SPICC26XXDMA_fxnTable, &spiCC26XXDMAObjects[1], &spiCC26XXDMAHWAttrs[1]},
    {NULL, NULL, NULL},
};

/*
 *  ======== Board_openSPI ========
 */
SPI_Handle Board_openSPI(UInt spiPortIndex, SPI_Params *spiParams)
{
    /* Initialize the SPI driver */
    /* By design, SPI_init() is idempotent */
    SPI_init();

    /* initialize the pins associated with the respective UART */
    switch(spiPortIndex) {
        case 0:
        case 1:
            break;

        default:
            return(NULL);
    }

    /* open the SPI port */
    return (SPI_open(spiPortIndex, spiParams));
}

/*
 *  ========================== SPI DMA end =====================================
 */

/*
 *  ============================= I2C Begin=====================================
 */
/* Place into subsections to allow the TI linker to remove items properly */
#if defined(__TI_COMPILER_VERSION__)
#pragma DATA_SECTION(I2C_config, ".const:I2C_config")
#pragma DATA_SECTION(i2cCC26xxHWAttrs, ".const:i2cCC26xxHWAttrs")
#endif

/* Include drivers */
#include <ti/drivers/i2c/I2CCC26XX.h>

/* I2C objects */
I2CCC26XX_Object i2cCC26xxObjects[CC1350_I2CCOUNT];

/* I2C configuration structure, describing which pins are to be used */
const I2CCC26XX_HWAttrsV1 i2cCC26xxHWAttrs[CC1350_I2CCOUNT] = {
    {
        .baseAddr = I2C0_BASE,
        .powerMngrId = PowerCC26XX_PERIPH_I2C0,
        .intNum = INT_I2C_IRQ,    /* <driverlib>/inc/hw_ints.h */
        .intPriority = (~0),
        .swiPriority = 0,
        .sdaPin = Board_I2C0_SDA0,
        .sclPin = Board_I2C0_SCL0,
    },
    {
        .baseAddr = I2C0_BASE,
        .powerMngrId = PowerCC26XX_PERIPH_I2C0,
        .intNum = INT_I2C_IRQ,    /* <driverlib>/inc/hw_ints.h */
        .intPriority = (~0),
        .swiPriority = 0,
        .sdaPin = Board_I2C0_SDA1,
        .sclPin = Board_I2C0_SCL1,
    }
};

const I2C_Config I2C_config[] = {
    {
        .fxnTablePtr = &I2CCC26XX_fxnTable,
        .object = &i2cCC26xxObjects[0],
        .hwAttrs = &i2cCC26xxHWAttrs[0]
    },
    {
        .fxnTablePtr = &I2CCC26XX_fxnTable,
        .object = &i2cCC26xxObjects[1],
        .hwAttrs = &i2cCC26xxHWAttrs[1]
    },
    {NULL, NULL, NULL}
};

/*
 *  ======== Board_openI2C ========
 *  Initialize the I2C driver.
 *  Initialize the I2C port's pins.
 *  Open the I2C port.
 */
I2C_Handle Board_openI2C(UInt i2cPortIndex, I2C_Params *i2cParams)
{
    
    /* Initialize the I2C driver */
    /* By design, I2C_init() is idempotent */
    I2C_init();
    
    /* initialize the pins associated with the respective I2C */
    switch(i2cPortIndex) {
        case 0:
        case 1:
            break;

        default:
            return (NULL);
    }

    /* open the I2C */
    return (I2C_open(i2cPortIndex, i2cParams));
}

/*
 *  ========================== I2C end =========================================
*/

/*
 *  ========================== Crypto begin =======================================
 *  NOTE: The Crypto implementation should be considered experimental and not validated!
*/
/* Place into subsections to allow the TI linker to remove items properly */
#if defined(__TI_COMPILER_VERSION__)
#pragma DATA_SECTION(CryptoCC26XX_config, ".const:CryptoCC26XX_config")
#pragma DATA_SECTION(cryptoCC26XXHWAttrs, ".const:cryptoCC26XXHWAttrs")
#endif

/* Include drivers */
#include <ti/drivers/crypto/CryptoCC26XX.h>

/* Crypto objects */
CryptoCC26XX_Object cryptoCC26XXObjects[CC1350_CRYPTOCOUNT];

/* Crypto configuration structure, describing which pins are to be used */
const CryptoCC26XX_HWAttrs cryptoCC26XXHWAttrs[CC1350_CRYPTOCOUNT] = {
    {
        .baseAddr = CRYPTO_BASE,
        .powerMngrId = PowerCC26XX_PERIPH_CRYPTO,
        .intNum = INT_CRYPTO_RESULT_AVAIL_IRQ,    /* <driverlib>/inc/hw_ints.h */
        .intPriority = ~0,
    }
};

/* Crypto configuration structure */
const CryptoCC26XX_Config CryptoCC26XX_config[] = {
    {
        .object  = &cryptoCC26XXObjects[0],
        .hwAttrs = &cryptoCC26XXHWAttrs[0]
    },
    {NULL, NULL}
};

/*
 *  ============================ GPTimer begin =================================
 *  Remove unused entries to reduce flash usage both in Board.c and Board.h
 */

#include <ti/drivers/timer/GPTimerCC26XX.h>

/* Place into subsections to allow the TI linker to remove items properly */
#if defined(__TI_COMPILER_VERSION__)
#pragma DATA_SECTION(GPTimerCC26XX_config, ".const:GPTimerCC26XX_config")
#pragma DATA_SECTION(gptimerCC26xxHWAttrs, ".const:gptimerCC26xxHWAttrs")
#endif

/* GPTimer hardware attributes, one per timer part (Timer 0A, 0B, 1A, 1B..) */
const GPTimerCC26XX_HWAttrs gptimerCC26xxHWAttrs[CC1350STK_GPTIMERPARTSCOUNT] = {
    { .baseAddr = GPT0_BASE, .intNum = INT_GPT0A, .intPriority = (~0), .powerMngrId = PowerCC26XX_PERIPH_GPT0, .pinMux = GPT_PIN_0A, },
    { .baseAddr = GPT0_BASE, .intNum = INT_GPT0B, .intPriority = (~0), .powerMngrId = PowerCC26XX_PERIPH_GPT0, .pinMux = GPT_PIN_0B, },
    { .baseAddr = GPT1_BASE, .intNum = INT_GPT1A, .intPriority = (~0), .powerMngrId = PowerCC26XX_PERIPH_GPT1, .pinMux = GPT_PIN_1A, },
    { .baseAddr = GPT1_BASE, .intNum = INT_GPT1B, .intPriority = (~0), .powerMngrId = PowerCC26XX_PERIPH_GPT1, .pinMux = GPT_PIN_1B, },
    { .baseAddr = GPT2_BASE, .intNum = INT_GPT2A, .intPriority = (~0), .powerMngrId = PowerCC26XX_PERIPH_GPT2, .pinMux = GPT_PIN_2A, },
    { .baseAddr = GPT2_BASE, .intNum = INT_GPT2B, .intPriority = (~0), .powerMngrId = PowerCC26XX_PERIPH_GPT2, .pinMux = GPT_PIN_2B, },
    { .baseAddr = GPT3_BASE, .intNum = INT_GPT3A, .intPriority = (~0), .powerMngrId = PowerCC26XX_PERIPH_GPT3, .pinMux = GPT_PIN_3A, },
    { .baseAddr = GPT3_BASE, .intNum = INT_GPT3B, .intPriority = (~0), .powerMngrId = PowerCC26XX_PERIPH_GPT3, .pinMux = GPT_PIN_3B, },
};

/*  GPTimer objects, one per full-width timer (A+B) (Timer 0, Timer 1..) */
GPTimerCC26XX_Object gptimerCC26XXObjects[CC1350STK_GPTIMERCOUNT];

/* GPTimer configuration (used as GPTimer_Handle by driver and application) */
const GPTimerCC26XX_Config GPTimerCC26XX_config[CC1350STK_GPTIMERPARTSCOUNT] = {
    { &gptimerCC26XXObjects[0], &gptimerCC26xxHWAttrs[0], GPT_A },
    { &gptimerCC26XXObjects[0], &gptimerCC26xxHWAttrs[1], GPT_B },
    { &gptimerCC26XXObjects[1], &gptimerCC26xxHWAttrs[2], GPT_A },
    { &gptimerCC26XXObjects[1], &gptimerCC26xxHWAttrs[3], GPT_B },
    { &gptimerCC26XXObjects[2], &gptimerCC26xxHWAttrs[4], GPT_A },
    { &gptimerCC26XXObjects[2], &gptimerCC26xxHWAttrs[5], GPT_B },
    { &gptimerCC26XXObjects[3], &gptimerCC26xxHWAttrs[6], GPT_A },
    { &gptimerCC26XXObjects[3], &gptimerCC26xxHWAttrs[7], GPT_B },
};

/*
 *  ============================ GPTimer end ===================================
 */

/*
 *  ============================= PWM begin ====================================
 *  Remove unused entries to reduce flash usage both in Board.c and Board.h
 */

#include <ti/drivers/PWM.h>
#include <ti/drivers/pwm/PWMTimerCC26XX.h>
#include <driverlib/timer.h>

/* !!! Hack to workaround unilateral 'const' modifier in CC26xx HwAttrs typedef !!! */
typedef struct myPWMTimerCC26XX_HwAttrs
{
    PIN_Id  pwmPin;               /*!< PIN to output PWM signal on */
    uint8_t gpTimerUnit;          /*!< GPTimer unit index (0A, 0B, 1A..) */
} myPWMTimerCC26XX_HwAttrs;


/* Place into subsections to allow the TI linker to remove items properly */
#if defined(__TI_COMPILER_VERSION__)
#pragma DATA_SECTION(PWM_config, ".const:PWM_config")
#pragma DATA_SECTION(pwmtimerCC26xxHWAttrs, ".const:pwmtimerCC26xxHWAttrs")
#endif

/* PWM configuration, one per PWM output.   */
myPWMTimerCC26XX_HwAttrs pwmtimerCC26xxHWAttrs[CC1350STK_PWMCOUNT] = {
    { .pwmPin = Board_PWMPIN0, .gpTimerUnit = Board_GPTIMER0A },
    { .pwmPin = Board_PWMPIN1, .gpTimerUnit = Board_GPTIMER0B },
    { .pwmPin = Board_PWMPIN2, .gpTimerUnit = Board_GPTIMER1A },
    { .pwmPin = Board_PWMPIN3, .gpTimerUnit = Board_GPTIMER1B },
    { .pwmPin = Board_PWMPIN4, .gpTimerUnit = Board_GPTIMER2A },
    { .pwmPin = Board_PWMPIN5, .gpTimerUnit = Board_GPTIMER2B },
    { .pwmPin = Board_PWMPIN6, .gpTimerUnit = Board_GPTIMER3A },
    { .pwmPin = Board_PWMPIN7, .gpTimerUnit = Board_GPTIMER3B },
};

/* PWM object, one per PWM output */
PWMTimerCC26XX_Object pwmtimerCC26xxObjects[CC1350STK_PWMCOUNT];

extern const PWM_FxnTable PWMTimerCC26XX_fxnTable;

/* PWM configuration (used as PWM_Handle by driver and application) */
const PWM_Config PWM_config[CC1350STK_PWMCOUNT + 1] = {
    { &PWMTimerCC26XX_fxnTable, &pwmtimerCC26xxObjects[0], &pwmtimerCC26xxHWAttrs[0] },
    { &PWMTimerCC26XX_fxnTable, &pwmtimerCC26xxObjects[1], &pwmtimerCC26xxHWAttrs[1] },
    { &PWMTimerCC26XX_fxnTable, &pwmtimerCC26xxObjects[2], &pwmtimerCC26xxHWAttrs[2] },
    { &PWMTimerCC26XX_fxnTable, &pwmtimerCC26xxObjects[3], &pwmtimerCC26xxHWAttrs[3] },
    { &PWMTimerCC26XX_fxnTable, &pwmtimerCC26xxObjects[4], &pwmtimerCC26xxHWAttrs[4] },
    { &PWMTimerCC26XX_fxnTable, &pwmtimerCC26xxObjects[5], &pwmtimerCC26xxHWAttrs[5] },
    { &PWMTimerCC26XX_fxnTable, &pwmtimerCC26xxObjects[6], &pwmtimerCC26xxHWAttrs[6] },
    { &PWMTimerCC26XX_fxnTable, &pwmtimerCC26xxObjects[7], &pwmtimerCC26xxHWAttrs[7] },
    { NULL,                NULL,                 NULL                 }
};

/*
 *  ============================= PWM end ======================================
 */

/*
 *  ========================== ADC begin =========================================
 */
/* Place into subsections to allow the TI linker to remove items properly */
#if defined(__TI_COMPILER_VERSION__)
#pragma DATA_SECTION(ADC_config, ".const:ADC_config")
#pragma DATA_SECTION(adcCC26xxHWAttrs, ".const:adcCC26xxHWAttrs")
#endif

/* Include drivers */
#include <ti/drivers/ADC.h>
#include <ti/drivers/adc/ADCCC26XX.h>

/* ADC objects */
ADCCC26XX_Object adcCC26xxObjects[CC1350STK_ADCCOUNT];


const ADCCC26XX_HWAttrs adcCC26xxHWAttrs[CC1350STK_ADCCOUNT] = {
    {
        .adcDIO = Board_DIO23_ANALOG,
        .adcCompBInput = ADC_COMPB_IN_AUXIO7,
        .refSource = ADCCC26XX_FIXED_REFERENCE,
        .samplingDuration = ADCCC26XX_SAMPLING_DURATION_2P7_US,
        .inputScalingEnabled = true,
        .triggerSource = ADCCC26XX_TRIGGER_MANUAL
    },
    {
        .adcDIO = Board_DIO24_ANALOG,
        .adcCompBInput = ADC_COMPB_IN_AUXIO6,
        .refSource = ADCCC26XX_FIXED_REFERENCE,
        .samplingDuration = ADCCC26XX_SAMPLING_DURATION_2P7_US,
        .inputScalingEnabled = true,
        .triggerSource = ADCCC26XX_TRIGGER_MANUAL
    },
    {
        .adcDIO = Board_DIO25_ANALOG,
        .adcCompBInput = ADC_COMPB_IN_AUXIO5,
        .refSource = ADCCC26XX_FIXED_REFERENCE,
        .samplingDuration = ADCCC26XX_SAMPLING_DURATION_2P7_US,
        .inputScalingEnabled = true,
        .triggerSource = ADCCC26XX_TRIGGER_MANUAL
    },
    {
        .adcDIO = Board_DIO26_ANALOG,
        .adcCompBInput = ADC_COMPB_IN_AUXIO4,
        .refSource = ADCCC26XX_FIXED_REFERENCE,
        .samplingDuration = ADCCC26XX_SAMPLING_DURATION_2P7_US,
        .inputScalingEnabled = true,
        .triggerSource = ADCCC26XX_TRIGGER_MANUAL
    },
    {
        .adcDIO = Board_DIO27_ANALOG,
        .adcCompBInput = ADC_COMPB_IN_AUXIO3,
        .refSource = ADCCC26XX_FIXED_REFERENCE,
        .samplingDuration = ADCCC26XX_SAMPLING_DURATION_2P7_US,
        .inputScalingEnabled = true,
        .triggerSource = ADCCC26XX_TRIGGER_MANUAL
    },
    {
        .adcDIO = Board_DIO28_ANALOG,
        .adcCompBInput = ADC_COMPB_IN_AUXIO2,
        .refSource = ADCCC26XX_FIXED_REFERENCE,
        .samplingDuration = ADCCC26XX_SAMPLING_DURATION_2P7_US,
        .inputScalingEnabled = true,
        .triggerSource = ADCCC26XX_TRIGGER_MANUAL
    },
    {
        .adcDIO = Board_DIO29_ANALOG,
        .adcCompBInput = ADC_COMPB_IN_AUXIO1,
        .refSource = ADCCC26XX_FIXED_REFERENCE,
        .samplingDuration = ADCCC26XX_SAMPLING_DURATION_2P7_US,
        .inputScalingEnabled = true,
        .triggerSource = ADCCC26XX_TRIGGER_MANUAL
    },
    {
        .adcDIO = Board_DIO30_ANALOG,
        .adcCompBInput = ADC_COMPB_IN_AUXIO0,
        .refSource = ADCCC26XX_FIXED_REFERENCE,
        .samplingDuration = ADCCC26XX_SAMPLING_DURATION_10P9_MS,
        .inputScalingEnabled = true,
        .triggerSource = ADCCC26XX_TRIGGER_MANUAL
    },
    {
        .adcDIO = PIN_UNASSIGNED,
        .adcCompBInput = ADC_COMPB_IN_DCOUPL,
        .refSource = ADCCC26XX_FIXED_REFERENCE,
        .samplingDuration = ADCCC26XX_SAMPLING_DURATION_2P7_US,
        .inputScalingEnabled = true,
        .triggerSource = ADCCC26XX_TRIGGER_MANUAL
    },
    {
        .adcDIO = PIN_UNASSIGNED,
        .adcCompBInput = ADC_COMPB_IN_VSS,
        .refSource = ADCCC26XX_FIXED_REFERENCE,
        .samplingDuration = ADCCC26XX_SAMPLING_DURATION_2P7_US,
        .inputScalingEnabled = true,
        .triggerSource = ADCCC26XX_TRIGGER_MANUAL
    },
    {
        .adcDIO = PIN_UNASSIGNED,
        .adcCompBInput = ADC_COMPB_IN_VDDS,
        .refSource = ADCCC26XX_FIXED_REFERENCE,
        .samplingDuration = ADCCC26XX_SAMPLING_DURATION_2P7_US,
        .inputScalingEnabled = true,
        .triggerSource = ADCCC26XX_TRIGGER_MANUAL
    }
};

const ADC_Config ADC_config[] = {
    {&ADCCC26XX_fxnTable, &adcCC26xxObjects[0], &adcCC26xxHWAttrs[0]},
    {&ADCCC26XX_fxnTable, &adcCC26xxObjects[1], &adcCC26xxHWAttrs[1]},
    {&ADCCC26XX_fxnTable, &adcCC26xxObjects[2], &adcCC26xxHWAttrs[2]},
    {&ADCCC26XX_fxnTable, &adcCC26xxObjects[3], &adcCC26xxHWAttrs[3]},
    {&ADCCC26XX_fxnTable, &adcCC26xxObjects[4], &adcCC26xxHWAttrs[4]},
    {&ADCCC26XX_fxnTable, &adcCC26xxObjects[5], &adcCC26xxHWAttrs[5]},
    {&ADCCC26XX_fxnTable, &adcCC26xxObjects[6], &adcCC26xxHWAttrs[6]},
    {&ADCCC26XX_fxnTable, &adcCC26xxObjects[7], &adcCC26xxHWAttrs[7]},
    {&ADCCC26XX_fxnTable, &adcCC26xxObjects[8], &adcCC26xxHWAttrs[8]},
    {&ADCCC26XX_fxnTable, &adcCC26xxObjects[9], &adcCC26xxHWAttrs[9]},
    {&ADCCC26XX_fxnTable, &adcCC26xxObjects[10], &adcCC26xxHWAttrs[10]},
    {NULL, NULL, NULL},
};

/*
 *  ========================== ADC end =========================================
 */

/* 
 *  =============================== Power ===============================
 */

#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26XX.h>

/*
 *  ======== PowerCC26XX_config ========
 */
const PowerCC26XX_Config PowerCC26XX_config = {
    .policyInitFxn = NULL,
    .policyFxn = PowerCC26XX_standbyPolicy,
    .calibrateFxn = PowerCC26XX_calibrate,
    .enablePolicy = TRUE,
    .calibrateRCOSC_LF = TRUE,
    .calibrateRCOSC_HF = TRUE
};

/*
 *  ======== Board_initPWM ========
 */
void Board_initPWM(void)
{
    PWM_init();
}

/*
 *  ======== Board_initPIN ========
 */
void Board_initPIN()
{
    PIN_init(BoardGpioInitTable);
}

/*
 *  ======== Board_initPower ========
 */
void Board_initPower(void)
{
    Power_init();
}

/*
 *  ======== Board_init ========
 *  Initialialize the ti.platforms.tink2 hardware
 */
void Board_init(void)
{
    /* driver-independent initialization */

    /* driver-specific initialization */
    Board_initPower();
    Board_initPIN();
    Board_initGPIO();
    Board_initPWM();
}
