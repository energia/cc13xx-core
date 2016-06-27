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
 *  @file       I2SCC3200DMA.h
 *
 *  @brief      I2S driver implementation for a CC3200 I2S controller
 *
 *  The I2S header file should be included in an application as follows:
 *  @code
 *  #include <ti/drivers/I2S.h>
 *  #include <ti/drivers/i2s/I2SCC3200DMA.h>
 *  @endcode
 *
 *  Refer to @ref I2S.h for a complete description of APIs & example of use.
 *
 *  ============================================================================
 */

#ifndef ti_drivers_i2s_I2SCC3200DMA__include
#define ti_drivers_i2s_I2SCC3200DMA__include

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <ti/drivers/I2S.h>
#include <ti/drivers/ports/HwiP.h>
#include <ti/drivers/ports/SemaphoreP.h>
#include <ti/drivers/utils/List.h>
#include <ti/drivers/dma/UDMACC3200.h>

/**
 *  @addtogroup I2S_STATUS
 *  I2SCC3200DMA_STATUS_* macros are command codes only defined in the
 *  I2SCC3200DMA.h driver implementation and need to:
 *  @code
 *  #include <ti/drivers/i2s/I2SCC3200DMA.h>
 *  @endcode
 *  @{
 */

/* Add I2SCC3200DMA_STATUS_* macros here */

/** @}*/

/**
 *  @addtogroup I2S_CMD
 *  I2SCC3200DMA_CMD_* macros are command codes only defined in the
 *  I2SCC3200DMA.h driver implementation and need to:
 *  @code
 *  #include <ti/drivers/i2s/I2SCC3200DMA.h>
 *  @endcode
 *  @{
 */

#define I2SCC3200DMA_CMD_SET_ZEROBUF_LEN      I2S_CMD_RESERVED + 0
#define I2SCC3200DMA_CMD_SET_EMPTYBUF_LEN     I2S_CMD_RESERVED + 1

/** @}*/

/* BACKWARDS COMPATIBILITY */
#define I2SCC3200DMA_SET_ZEROBUF_LEN      I2SCC3200DMA_CMD_SET_ZEROBUF_LEN
#define I2SCC3200DMA_SET_EMPTYBUF_LEN     I2SCC3200DMA_CMD_SET_EMPTYBUF_LEN
/* END BACKWARDS COMPATIBILITY */

/* Value for Invalid Index */
#define I2SCC3200DMA_INDEX_INVALID      0xFF

/*Number of Serial data pins supported*/
#define I2SCC3200DMA_NUM_SERIAL_PINS    2

/*!
 *  @brief
 *  I2SCC3200DMA data size is used to determine how to configure the
 *  DMA data transfers. This field is to be only used internally.
 *
 *  I2SCC3200DMA_16bit: txBuf and rxBuf are arrays of uint16_t elements
 *  I2SCC3200DMA_32bit: txBuf and rxBuf are arrays of uint32_t elements
 */
typedef enum I2SCC3200DMA_DataSize {
    I2SCC3200DMA_16bit = 0,
    I2SCC3200DMA_32bit = 1
} I2SCC3200DMA_DataSize;


/* I2S function table pointer */
extern const I2S_FxnTable I2SCC3200DMA_fxnTable;


/*!
 *  @brief      I2SCC3200DMA Hardware attributes
 *
 *  These fields, with the exception of intPriority,
 *  are used by driverlib APIs and therefore must be populated by
 *  driverlib macro definitions. For CC3200Ware these definitions are found in:
 *      - inc/hw_memmap.h
 *      - inc/hw_ints.h
 *
 *  intPriority is the I2S peripheral's interrupt priority, as defined by the
 *  underlying OS.  It is passed unmodified to the underlying OS's interrupt
 *  handler creation code, so you need to refer to the OS documentation
 *  for usage.  For example, for SYS/BIOS applications, refer to the
 *  ti.sysbios.family.arm.m3.Hwi documentation for SYS/BIOS usage of
 *  interrupt priorities.  If the driver uses the ti.drivers.ports interface
 *  instead of making OS calls directly, then the HwiP port handles the
 *  interrupt priority in an OS specific way.  In the case of the SYS/BIOS
 *  port, intPriority is passed unmodified to Hwi_create().
 *
 *  A sample structure is shown below:
 *  @code
 *  const I2SCC3200DMA_HWAttrs i2sCC3200HWAttrs[] = {
 *      {
 *          .baseAddr = I2S_BASE,
 *          .intNum = INT_I2S,
 *          .intPriority = (~0),
 *          .rxChannelIndex = UDMA_CH4_I2S_RX,
 *          .txChannelIndex = UDMA_CH5_I2S_TX
 *      }
 *  };
 *  @endcode
 */
typedef struct I2SCC3200DMA_HWAttrs {
    /*! I2S Peripheral's base address */
    uint32_t baseAddr;
    /*! I2S Peripheral's interrupt vector */
    uint32_t intNum;
    /*! I2S Peripheral's interrupt priority */
    uint32_t intPriority;
    /*! uDMA controlTable receive channel index */
    unsigned long rxChannelIndex;
    /*! uDMA controlTable transmit channel index */
    unsigned long txChannelIndex;
} I2SCC3200DMA_HWAttrs;

/*!
 *  @brief    CC3200 Serial Pin Configuration
 */
typedef struct I2SCC3200DMA_SerialPinConfig {
    /*!< Pin number  */
    unsigned char         pinNumber;

    /*!< Mode the pin will operate(Rx/Tx) */
    I2S_PinMode           pinMode;

     /*!< Pin configuration in inactive state */
    I2S_SerInActiveConfig inActiveConfig;

} I2SCC3200DMA_SerialPinConfig;

/*!
 *  @brief    CC3200 specific I2S Parameters
 */
typedef struct I2SCC3200DMA_SerialPinParams {

    /*!< CC3200 Serial Pin Configuration */
    I2SCC3200DMA_SerialPinConfig serialPinConfig[I2SCC3200DMA_NUM_SERIAL_PINS];

} I2SCC3200DMA_SerialPinParams;

/*!
 *  @brief      I2SCC3200DMA Serial pin variables
 *
 *  The application must not access any member variables of this structure!
 */
typedef struct I2SCC3200DMA_SerialPinVars {
    I2S_DataMode        readWriteMode;
    /* Pointer to read/write callback */
    I2S_Callback        readWriteCallback;
    /* Timeout for read/write semaphore */
    uint32_t            readWriteTimeout;

}I2SCC3200DMA_SerialPinVars;

/*!
 *  @brief      I2SCC3200DMA Object
 *
 *  The application must not access any member variables of this structure!
 */
typedef struct I2SCC3200DMA_Object {
    /* I2S control variables */
    bool                   opened;              /* Has the obj been opened */
    uint32_t               operationMode;       /* Mode of operation of I2S */

    /* I2S serial pin variables */
    I2SCC3200DMA_SerialPinVars serialPinVars[I2SCC3200DMA_NUM_SERIAL_PINS];

    uint16_t               readIndex;           /* read channel Index */
    uint16_t               writeIndex;          /* write channel Index */

    I2SCC3200DMA_DataSize  dmaSize;             /* Config DMA word size  */

    /* I2S OSAL objects */
    SemaphoreP_Handle      writeSem;            /* I2S write semaphore*/
    SemaphoreP_Handle      readSem;             /* I2S read semaphore */
    HwiP_Handle            hwiHandle;

    /*!< Length of zero buffer to write in case of no data */
    unsigned long          zeroWriteBufLength;

    /*!< Length of empty buffer to read in case of no data
         requested */
    unsigned long          emptyReadBufLength;

    /* Current Write buffer descriptor pointer */
    I2S_BufDesc            *CurrentWriteBufDesc;

    /* Current Read buffer descriptor pointer */
    I2S_BufDesc            *CurrentReadBufDesc;

    /* UDMA */
    UDMACC3200_Handle    dmaHandle;

    /* Lists for issue-reclaim mode */
    List_List             readActiveQueue;
    List_List             readDoneQueue;
    List_List             writeActiveQueue;
    List_List             writeDoneQueue;
} I2SCC3200DMA_Object, *I2SCC3200DMA_Handle;

/*!
 *  @brief  Function to initialize the I2S_Params struct to its defaults
 *
 * params->serialPinConfig[0].pinNumber         = 0;
 * params->serialPinConfig[0].pinMode           = I2S_PINMODE_RX;
 * params->serialPinConfig[0].inActiveConfig    = I2S_SERCONFIG_INACT_LOW_LEVEL;
 *
 * params->serialPinConfig[1].pinNumber         = 1;
 * params->serialPinConfig[1].pinMode           = I2S_PINMODE_TX;
 * params->serialPinConfig[1].inActiveConfig    = I2S_SERCONFIG_INACT_LOW_LEVEL;
 *
 *  @param  params  Parameter structure to initialize
 */
extern void I2SCC3200DMA_Params_init(I2SCC3200DMA_SerialPinParams *params);

#ifdef __cplusplus
}
#endif

#endif /* ti_drivers_i2s_I2SCC3200DMA__include */
