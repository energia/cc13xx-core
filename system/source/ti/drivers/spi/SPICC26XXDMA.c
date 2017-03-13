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

#include <xdc/runtime/Error.h>
#include <xdc/runtime/Assert.h>
#include <xdc/runtime/Diags.h>
#include <xdc/runtime/Log.h>
#include <xdc/runtime/System.h>
#include <xdc/runtime/Types.h>

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Swi.h>
#include <ti/sysbios/family/arm/m3/Hwi.h>
#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerCC26XX.h>

#include <ti/drivers/spi/SPICC26XXDMA.h>
#include <ti/drivers/dma/UDMACC26XX.h>

/* driverlib header files */
#ifdef DEVICE_FAMILY
    #undef DEVICE_FAMILY_PATH
    #define DEVICE_FAMILY_PATH(x) <ti/devices/DEVICE_FAMILY/x>
#else
    #error "You must define DEVICE_FAMILY at the project level as one of cc26x0, cc26x0r2, cc13x0, etc."
#endif

#include DEVICE_FAMILY_PATH(inc/hw_memmap.h)
#include DEVICE_FAMILY_PATH(inc/hw_ints.h)
#include DEVICE_FAMILY_PATH(inc/hw_types.h)
#include DEVICE_FAMILY_PATH(driverlib/ssi.h)
#include DEVICE_FAMILY_PATH(driverlib/sys_ctrl.h)
#include DEVICE_FAMILY_PATH(driverlib/udma.h)
#include DEVICE_FAMILY_PATH(driverlib/ioc.h)
#include DEVICE_FAMILY_PATH(driverlib/prcm.h)
#include DEVICE_FAMILY_PATH(driverlib/rom.h)

/* SPI test control register */
#define SSI_O_TCR               0x00000080
#define SSI_TCR_TESTFIFO_ENABLE        0x2
#define SSI_TCR_TESTFIFO_DISABLE       0x0
/* SPI test data register */
#define SSI_O_TDR               0x0000008C

/* Local typedef only. Used to easily migrate hwattrs versions */
typedef SPICC26XXDMA_HWAttrsV1 SPICC26XXDMA_HWAttrs;

/* Allocate space for DMA control table entries */
ALLOCATE_CONTROL_TABLE_ENTRY(dmaSpi0TxControlTableEntry, UDMA_CHAN_SSI0_TX);
ALLOCATE_CONTROL_TABLE_ENTRY(dmaSpi0RxControlTableEntry, UDMA_CHAN_SSI0_RX);
ALLOCATE_CONTROL_TABLE_ENTRY(dmaSpi1TxControlTableEntry, UDMA_CHAN_SSI1_TX);
ALLOCATE_CONTROL_TABLE_ENTRY(dmaSpi1RxControlTableEntry, UDMA_CHAN_SSI1_RX);

/* SPICC26XX functions */
void         SPICC26XXDMA_close(SPI_Handle handle);
int_fast16_t SPICC26XXDMA_control(SPI_Handle handle, uint_fast16_t cmd, void *arg);
void         SPICC26XXDMA_init(SPI_Handle handle);
SPI_Handle   SPICC26XXDMA_open(SPI_Handle handle, SPI_Params *params);
bool         SPICC26XXDMA_transfer(SPI_Handle handle, SPI_Transaction *transaction);
void         SPICC26XXDMA_transferCancel(SPI_Handle handle);

/* SPICC26XX internal functions */
static void SPICC26XXDMA_transferCallback(SPI_Handle handle, SPI_Transaction *msg);
static void SPICC26XXDMA_csnCallback(PIN_Handle handle, PIN_Id pinId);
static void SPICC26XXDMA_initHw(SPI_Handle handle);
static bool SPICC26XXDMA_initIO(SPI_Handle handle);
static void SPICC26XXDMA_flushFifos(SPI_Handle handle);

/* Internal power functions */
#ifdef SPICC26XXDMA_WAKEUP_ENABLED
static int spiPreNotify(unsigned int eventType, uintptr_t eventArg, uintptr_t clientArg);
#endif
static int spiPostNotify(unsigned int eventType, uintptr_t eventArg, uintptr_t clientArg);

/* SPI function table for SPICC26XXDMA implementation */
const SPI_FxnTable SPICC26XXDMA_fxnTable = {
    SPICC26XXDMA_close,
    SPICC26XXDMA_control,
    SPICC26XXDMA_init,
    SPICC26XXDMA_open,
    SPICC26XXDMA_transfer,
    SPICC26XXDMA_transferCancel
    };

/* Mapping SPI mode from generic driver to CC26XX driverlib */
static const uint32_t mode[] = {
    SSI_MODE_MASTER,    /* SPI_MASTER */
    SSI_MODE_SLAVE      /* SPI_SLAVE */
};

/* Mapping SPI frame format from generic driver to CC26XX driverlib */
static const uint32_t frameFormat[] = {
    SSI_FRF_MOTO_MODE_0,    /* SPI_POLO_PHA0 */
    SSI_FRF_MOTO_MODE_1,    /* SPI_POLO_PHA1 */
    SSI_FRF_MOTO_MODE_2,    /* SPI_POL1_PHA0 */
    SSI_FRF_MOTO_MODE_3,    /* SPI_POL1_PHA1 */
    SSI_FRF_TI,             /* SPI_TI */
    SSI_FRF_NMW             /* SPI_MW */
};

/*
 * This lookup table is used to configure the DMA channels for the appropriate
 * (8bit or 16bit) transfer sizes.
 * Table for an SPI DMA TX channel
 */
static const unsigned long dmaTxConfig[] = {
    UDMA_MODE_BASIC | UDMA_SIZE_8  | UDMA_SRC_INC_8  | UDMA_DST_INC_NONE | UDMA_ARB_4, /* 8bit  */
    UDMA_MODE_BASIC | UDMA_SIZE_16 | UDMA_SRC_INC_16 | UDMA_DST_INC_NONE | UDMA_ARB_4  /* 16bit */
};

/*
 * This lookup table is used to configure the DMA channels for the appropriate
 * (8bit or 16bit) transfer sizes.
 * Table for an SPI DMA RX channel
 */
static const unsigned long dmaRxConfig[] = {
    UDMA_MODE_BASIC | UDMA_SIZE_8  | UDMA_SRC_INC_NONE | UDMA_DST_INC_8  | UDMA_ARB_4, /* 8bit  */
    UDMA_MODE_BASIC | UDMA_SIZE_16 | UDMA_SRC_INC_NONE | UDMA_DST_INC_16 | UDMA_ARB_4  /* 16bit */
};

/*
 * This lookup table is used to configure the DMA channels for the appropriate
 * (8bit or 16bit) transfer sizes when either txBuf or rxBuf are NULL
 */
static const uint32_t dmaNullConfig[] = {
    UDMA_MODE_BASIC | UDMA_SIZE_8  | UDMA_SRC_INC_NONE | UDMA_DST_INC_NONE | UDMA_ARB_4, /* 8bit */
    UDMA_MODE_BASIC | UDMA_SIZE_16 | UDMA_SRC_INC_NONE | UDMA_DST_INC_NONE | UDMA_ARB_4  /* 16bit */
};

/*
 * Ensure safe setting of the standby disallow constraint.
 */
static inline void threadSafeConstraintSet(uint32_t txBufAddr, SPICC26XXDMA_Object *object) {
    unsigned int  key;

    /* Disable interrupts */
    key = Hwi_disable();

    if (!object->spiPowerConstraint) {
        /* Ensure flash is available if TX buffer is in flash. Flash starts with 0x0..*/
        if (((txBufAddr & 0xF0000000) == 0x0) && (txBufAddr)) {
            Power_setConstraint(PowerCC26XX_NEED_FLASH_IN_IDLE);
        }
        /* Set constraints to guarantee operation */
        Power_setConstraint(PowerCC26XX_SB_DISALLOW);
        object->spiPowerConstraint = true;
    }

    /* Re-enable interrupts */
    Hwi_restore(key);
}

/*
 * Ensure safe releasing of the standby disallow constraint.
 */
static inline void threadSafeConstraintRelease(uint32_t txBufAddr, SPICC26XXDMA_Object *object) {
    unsigned int  key;
    /* Disable interrupts */
    key = Hwi_disable();

    if (object->spiPowerConstraint) {
        /* Release need flash if buffer was in flash. */
        if (((txBufAddr & 0xF0000000) == 0x0) && (txBufAddr)) {
            Power_releaseConstraint(PowerCC26XX_NEED_FLASH_IN_IDLE);
        }
        /* Release standby constraint since operation is done. */
        Power_releaseConstraint(PowerCC26XX_SB_DISALLOW);
        object->spiPowerConstraint = false;
    }

    /* Re-enable interrupts */
    Hwi_restore(key);
}

/*!
 *  @brief  Function to close a given CC26XX SPI peripheral specified by the
 *          SPI handle.
 *
 *  Will disable the SPI, disable all SPI interrupts and release the
 *  dependency on the corresponding power domain.
 *
 *  @pre    SPICC26XXDMA_open() has to be called first.
 *          Calling context: Task
 *
 *  @param  handle  A SPI_Handle returned from SPI_open()
 *
 *  @sa     SPICC26XXDMA_open
 */
void SPICC26XXDMA_close(SPI_Handle handle)
{
    unsigned int                 key;
    SPICC26XXDMA_Object         *object;
    SPICC26XXDMA_HWAttrs const  *hwAttrs;

    /* Get the pointer to the object and hwAttrs */
    hwAttrs = handle->hwAttrs;
    object = handle->object;

    /* Release the uDMA dependency and potentially power down uDMA. */
    UDMACC26XX_close(object->udmaHandle);

    /* Deallocate pins */
    PIN_close(object->pinHandle);

    /* Disable the SPI */
    SSIDisable(hwAttrs->baseAddr);

    /* Destroy the Hwi */
    Hwi_destruct(&(object->hwi));

    /* Destroy the Swi */
    Swi_destruct(&(object->swi));

    /* Release power dependency on SPI. */
    Power_releaseDependency(hwAttrs->powerMngrId);

    if (object->transferMode == SPI_MODE_BLOCKING) {
        Semaphore_destruct(&(object->transferComplete));
    }

    /* Unregister power notification objects */
#ifdef SPICC26XXDMA_WAKEUP_ENABLED
    Power_unregisterNotify(&object->spiPreObj);
#endif
    Power_unregisterNotify(&object->spiPostObj);

    /* Mark the module as available */
    key = Hwi_disable();
    object->isOpen = false;
    Hwi_restore(key);

    Log_print1(Diags_USER1, "SPI:(%p) closed", hwAttrs->baseAddr);
}

/*!
 *  @brief  Function for setting control parameters of the SPI driver
 *          after it has been opened.
 *
 *  @pre    SPICC26XXDMA_open() has to be called first.
 *          Calling context: Hwi, Swi, Task
 *
 *  @param  handle A SPI handle returned from SPICC26XXDMA_open()
 *
 *  @param  cmd  The command to execute, supported commands are:
 *              | Command                               | Description             |
 *              |-------------------------------------- |-------------------------|
 *              | ::SPICC26XXDMA_CMD_RETURN_PARTIAL_ENABLE  | Enable RETURN_PARTIAL   |
 *              | ::SPICC26XXDMA_CMD_RETURN_PARTIAL_DISABLE | Disable RETURN_PARTIAL  |
 *              | ::SPICC26XXDMA_CMD_SET_CSN_PIN            | Re-configure chip select pin |
 *
 *  @param  *arg  Pointer to command arguments.
 *
 *  @return ::SPI_STATUS_SUCCESS if success, or error code if error.
 */
int_fast16_t SPICC26XXDMA_control(SPI_Handle handle, uint_fast16_t cmd, void *arg)
{
    SPICC26XXDMA_Object        *object;
    SPICC26XXDMA_HWAttrs const *hwAttrs;
    PIN_Config                  pinConfig;
    PIN_Id                      pinId;

    /* Get the pointer to the object and hwAttr */
    hwAttrs = handle->hwAttrs;
    object = handle->object;

    /* Initialize return value*/
    int ret = SPI_STATUS_ERROR;

    /* Perform command */
    switch(cmd) {
        case SPICC26XXDMA_CMD_RETURN_PARTIAL_ENABLE:
            /* Enable RETURN_PARTIAL if slave mode is enabled */

            if(object->mode == SPI_SLAVE){
                object->returnPartial = true;
                ret = SPI_STATUS_SUCCESS;
            }
            else{
                /* Partial return not available in master mode. */
                ret = SPI_STATUS_ERROR;
            }
            break;

        case SPICC26XXDMA_CMD_RETURN_PARTIAL_DISABLE:
            /* Disable RETURN_PARTIAL */
            object->returnPartial = false;
            ret = SPI_STATUS_SUCCESS;
            break;

        case SPICC26XXDMA_CMD_SET_CSN_PIN:
            pinId = ((*(PIN_Id *) arg));

            /* Configure CSN pin and remap PIN_ID to new CSN pin specified by arg */
            if (object->mode == SPI_SLAVE) {
                pinConfig = PIN_INPUT_EN | PIN_PULLUP | pinId;
            }
            else {
                pinConfig = PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_INPUT_DIS | PIN_DRVSTR_MED | pinId;
            }

            if (pinId != PIN_UNASSIGNED) {
                /* Attempt to add the new pin */
                if (PIN_add(object->pinHandle, pinConfig) == PIN_SUCCESS) {
                    /* Configure pin mux */
                    PINCC26XX_setMux(object->pinHandle, pinId,  (hwAttrs->baseAddr == SSI0_BASE ? IOC_PORT_MCU_SSI0_FSS : IOC_PORT_MCU_SSI1_FSS));

                    /* Remove old pin and revert to default setting specified in the board file */
                    PIN_remove(object->pinHandle, object->csnPin);

                    /* Keep track of current CSN pin */
                    object->csnPin = pinId;

                    /* Set return value to indicate success */
                    ret = SPI_STATUS_SUCCESS;
                }
            }
            else {
                /* We want to use software ctrl CSN. Hence, undo any prior hardware CSN pin muxing */
                /* Remove old pin and revert to default setting specified in the board file (implicitly sets IO muxing to GPIO mode) */
                PIN_remove(object->pinHandle, object->csnPin);

                /* Keep track of current CSN pin */
                object->csnPin = pinId;

                /* Set return value to indicate success */
                ret = SPI_STATUS_SUCCESS;
            }
            break;

#ifdef SPICC26XXDMA_WAKEUP_ENABLED
        case SPICC26XXDMA_CMD_SET_CSN_WAKEUP:
            /* Set wakeup callback function no matter what, NULL signals that the wakeup feature is disabled */
            object->wakeupCallbackFxn = *(SPICC26XXDMA_CallbackFxn)arg;
            ret = SPI_STATUS_SUCCESS;
            break;
#endif
        default:
            /* This command is not defined */
            ret = SPI_STATUS_UNDEFINEDCMD;
            break;
    }

    return (ret);
}

/*
 *  ======== SPICC26XXDMA_configDMA ========
 *  This functions configures the transmit and receive DMA channels for a given
 *  SPI_Handle and SPI_Transaction
 *
 *  @pre    Function assumes that the handle and transaction is not NULL
 */
static void SPICC26XXDMA_configDMA(SPI_Handle handle, SPI_Transaction *transaction)
{
    SPICC26XXDMA_Object             *object;
    SPICC26XXDMA_HWAttrs const      *hwAttrs;
    volatile tDMAControlTable       *dmaControlTableEntry;
    uint16_t                        numberOfBytes;

    /* Get the pointer to the object and hwAttrs */
    object = handle->object;
    hwAttrs = handle->hwAttrs;

    /* Calculate the number of bytes for the transfer */
    numberOfBytes = ((uint16_t)(transaction->count) << (object->frameSize));

    /* Setup RX side */
    /* Set pointer to Rx control table entry */
    dmaControlTableEntry = (hwAttrs->baseAddr == SSI0_BASE ? &dmaSpi0RxControlTableEntry : &dmaSpi1RxControlTableEntry);
    if (transaction->rxBuf) {
        dmaControlTableEntry->ui32Control = dmaRxConfig[object->frameSize];
        dmaControlTableEntry->pvDstEndAddr = (void *)((uint32_t)(transaction->rxBuf) + numberOfBytes - 1);
    }
    else {
        dmaControlTableEntry->ui32Control = dmaNullConfig[object->frameSize];
        dmaControlTableEntry->pvDstEndAddr = (void *) &(object->scratchBuf);
    }
    dmaControlTableEntry->pvSrcEndAddr = (void *)(hwAttrs->baseAddr + SSI_O_DR);
    dmaControlTableEntry->ui32Control |= UDMACC26XX_SET_TRANSFER_SIZE((uint16_t)transaction->count);

    /* Setup TX side */
    /* Set pointer to Tx control table entry */
    dmaControlTableEntry = (hwAttrs->baseAddr == SSI0_BASE ? &dmaSpi0TxControlTableEntry : &dmaSpi1TxControlTableEntry);
    if (transaction->txBuf) {
        dmaControlTableEntry->ui32Control = dmaTxConfig[object->frameSize];
        dmaControlTableEntry->pvSrcEndAddr = (void *)((uint32_t)(transaction->txBuf) + numberOfBytes - 1);
    }
    else {
        object->scratchBuf = hwAttrs->defaultTxBufValue;
        dmaControlTableEntry->ui32Control = dmaNullConfig[object->frameSize];
        dmaControlTableEntry->pvSrcEndAddr = (void *) &(object->scratchBuf);
    }
    dmaControlTableEntry->pvDstEndAddr = (void *)(hwAttrs->baseAddr + SSI_O_DR);
    dmaControlTableEntry->ui32Control |= UDMACC26XX_SET_TRANSFER_SIZE((uint16_t)transaction->count);

    /* Enable the channels */
    UDMACC26XX_channelEnable(object->udmaHandle, (hwAttrs->rxChannelBitMask) | (hwAttrs->txChannelBitMask));

    /* Enable the required DMA channels in the SPI module to start the transaction */
    SSIDMAEnable(hwAttrs->baseAddr, SSI_DMA_TX | SSI_DMA_RX);

    Log_print1(Diags_USER1,"SPI:(%p) DMA transfer enabled", hwAttrs->baseAddr);

    Log_print5(Diags_USER2,"SPI:(%p) DMA transaction: %p, "
                           "rxBuf: %p; txBuf: %p; Count: %d",
                            hwAttrs->baseAddr,
                            (UArg)transaction,
                            (UArg)transaction->rxBuf,
                            (UArg)transaction->txBuf,
                            (UArg)transaction->count);
}

/*
 *  ======== SPICC26XXDMA_hwiFxn ========
 *  HWI ISR for the SPI when we use the UDMA
 */
static void SPICC26XXDMA_hwiFxn (UArg arg) {
    SPICC26XXDMA_Object        *object;
    SPICC26XXDMA_HWAttrs const *hwAttrs;
    uint32_t                    intStatus;

    /* Get the pointer to the object and hwAttrs */
    object = ((SPI_Handle)arg)->object;
    hwAttrs = ((SPI_Handle)arg)->hwAttrs;

    Log_print1(Diags_USER2, "SPI:(%p) hwi interrupt context start", hwAttrs->baseAddr);

    /* Get the interrupt status of the SPI controller
    */
    intStatus = SSIIntStatus(hwAttrs->baseAddr, true);
    SSIIntClear(hwAttrs->baseAddr, intStatus);

    /* Error handling:
     * Overrun in the RX Fifo -> at least one sample in the shift
     * register has been discarded  */
    if (intStatus & SSI_RXOR) {
        /* disable the interrupt */
        SSIIntDisable(hwAttrs->baseAddr, SSI_RXOR);

        /* If the RX overrun occurred during a transfer */
        if (object->currentTransaction) {
            /* Then cancel the ongoing transfer */
            SPICC26XXDMA_transferCancel((SPI_Handle)arg);
        }
        else {
            /* Otherwise disable the SPI and DMA modules and flush FIFOs */
            SSIDisable(hwAttrs->baseAddr);

            /* Disable SPI TX/RX DMA and clear DMA done interrupt just in case it finished */
            SSIDMADisable(hwAttrs->baseAddr, SSI_DMA_TX | SSI_DMA_RX);
            UDMACC26XX_clearInterrupt(object->udmaHandle, (hwAttrs->rxChannelBitMask) | (hwAttrs->txChannelBitMask));

            /* Clear out the FIFO by resetting SPI module and re-initting */
            SPICC26XXDMA_flushFifos((SPI_Handle)arg);
        }
        Log_print1(Diags_USER1, "RX FIFO overrun occurred in SPI: (%p) !\n", hwAttrs->baseAddr);
    }
    else {
        /*
         * Determine if the TX DMA channel has completed... In SPI slave mode
         * this interrupt may occur immediately (without the RX DMA channel).
         *
         * All transfers will set up both TX and RX DMA channels and both will finish.
         * Even if the transaction->rxBuf == NULL, it will setup a dummy RX transfer to
         * a scratch memory location which is then discarded.
         */
        if (UDMACC26XX_channelDone(object->udmaHandle, hwAttrs->txChannelBitMask)) {
            /* Disable SPI TX DMA and clear DMA done interrupt. */
            SSIDMADisable(hwAttrs->baseAddr, SSI_DMA_TX);
            UDMACC26XX_clearInterrupt(object->udmaHandle, hwAttrs->txChannelBitMask);
        }

        /*
         * Determine if the RX DMA channel has completed... In slave mode this interrupt
         * occurrence depends on when the SPI master starts sending data.
         */
        if (UDMACC26XX_channelDone(object->udmaHandle, hwAttrs->rxChannelBitMask)) {
            /* Disable SPI RX DMA and clear DMA done interrupt. */
            SSIDMADisable(hwAttrs->baseAddr, SSI_DMA_RX);
            UDMACC26XX_clearInterrupt(object->udmaHandle, hwAttrs->rxChannelBitMask);

            /* Transaction is complete */
            if (object->currentTransaction && (object->currentTransaction->status == SPI_TRANSFER_STARTED)) {
                object->currentTransaction->status = SPI_TRANSFER_COMPLETED;
            }

            /* Post SWI to handle remaining clean up and invocation of callback */
            Swi_post(Swi_handle(&(object->swi)));
        }
    }

    Log_print1(Diags_USER2, "SPI:(%p) hwi interrupt context end",
                             hwAttrs->baseAddr);
}

/*
 *  ======== SPICC26XXDMA_swiFxn ========
 *  SWI ISR for the SPI when we use the UDMA
 *  Determine if the RX DMA channel has completed... In slave mode this interrupt
 *  occurrence depends on when the SPI master starts sending data.
 */
static void SPICC26XXDMA_swiFxn (UArg arg0, UArg arg1) {
    SPI_Transaction         *transaction;
    SPICC26XXDMA_Object     *object;

    /* Get the pointer to the object and hwAttrs */
    object = ((SPI_Handle)arg0)->object;

    Log_print1(Diags_USER2, "SPI:(%p) swi interrupt context start",
                             ((SPICC26XXDMA_HWAttrs const *)(((SPI_Handle)arg0)->hwAttrs))->baseAddr);


    /* Check if there is an active transaction */
    if (object->currentTransaction == NULL) {
        /* If the currentTransaction is NULL, this SWI was posted already by either the ISR or transferCancel.
         * Do nothing and return.
         * There is no need to disable interrupts and set up a temporary pointer for this fxn. The only place
         * currentTransaction can be set after initialisation is in this function and this swi cannot pre-empt
         * itself to cause a race condition in the middle of it.
         */
        Log_print0(Diags_USER2, "SPI: transaction already completed by hwi or transferCancel");
        return;
    }

    /* Use a temporary transaction pointer in case the callback function
     * attempts to perform another SPI_transfer call
     */
    transaction = object->currentTransaction;

    /* Indicate we are done with this transfer */
    object->currentTransaction = NULL;

    Log_print2(Diags_USER1,"SPI:(%p) DMA transaction: %p complete",
                            ((SPICC26XXDMA_HWAttrs const  *)(((SPI_Handle)arg0)->hwAttrs))->baseAddr, (UArg)transaction);

    /* Release constraint since transaction is done */
    threadSafeConstraintRelease((uint32_t)(transaction->txBuf), object);

    /* Perform callback */
    object->transferCallbackFxn((SPI_Handle)arg0, transaction);

    Log_print1(Diags_USER2, "SPI:(%p) swi interrupt context end",
                             ((SPICC26XXDMA_HWAttrs const  *)(((SPI_Handle)arg0)->hwAttrs))->baseAddr);
}

/*
 *  ======== SPICC26XXDMA_flushTxFifo ========
 */
void SPICC26XXDMA_flushFifos(SPI_Handle handle) {

    /* Locals */
    SPICC26XXDMA_HWAttrs const  *hwAttrs;

    /* Get the pointer to the hwAttrs */
    hwAttrs = handle->hwAttrs;

    /* Flush RX FIFO */
    while(HWREG(hwAttrs->baseAddr + SSI_O_SR) & SSI_RX_NOT_EMPTY) {
        /* Read element from RX FIFO and discard */
        HWREG(hwAttrs->baseAddr + SSI_O_DR);
    }

    /* Enable TESTFIFO mode */
    HWREG(hwAttrs->baseAddr + SSI_O_TCR) = SSI_TCR_TESTFIFO_ENABLE;

    /* Flush TX FIFO */
    while(!(HWREG(hwAttrs->baseAddr + SSI_O_SR) & SSI_TX_EMPTY)) {
        /* Read element from TX FIFO and discard */
        HWREG(hwAttrs->baseAddr + SSI_O_TDR);
    }

    /* Disable TESTFIFO mode */
    HWREG(hwAttrs->baseAddr + SSI_O_TCR) = SSI_TCR_TESTFIFO_DISABLE;
}

/*!
 *  @brief SPI CC26XX initialization
 *
 *  The function will set the isOpen flag to false.
 *
 *  @pre    Calling context: Hwi, Swi, Task, Main
 *
 *  @param handle  A SPI_Handle
 *
 */
void SPICC26XXDMA_init(SPI_Handle handle)
{
    SPICC26XXDMA_Object         *object;

    /* Get the pointer to the object */
    object = handle->object;

    /* Mark the object as available */
    object->isOpen = false;

    /* Init power constraint flag. */
    object->spiPowerConstraint = false;
}

/*!
 *  @brief  Function to initialize the CC26XX SPI peripheral specified by the
 *          particular handle. The parameter specifies which mode the SPI
 *          will operate.
 *
 *  The function will set a dependency on it power domain, i.e. power up the
 *  module and enable the clock. The IOs are allocated. Neither the SPI nor UDMA module
 *  will be enabled.
 *
 *  @pre    SPI controller has been initialized.
 *          Calling context: Task
 *
 *  @param  handle        A SPI_Handle
 *
 *  @param  params        Pointer to a parameter block, if NULL it will use
 *                        default values
 *
 *  @return A SPI_Handle on success or a NULL on an error or if it has been
 *          already opened
 *
 *  @sa     SPICC26XXDMA_close()
 */
SPI_Handle SPICC26XXDMA_open(SPI_Handle handle, SPI_Params *params)
{
    /* Use union to save on stack allocation */
    union {
        Semaphore_Params        semParams;
        Hwi_Params              hwiParams;
        Swi_Params              swiParams;
    } paramsUnion;
    SPICC26XXDMA_Object        *object;
    SPICC26XXDMA_HWAttrs const *hwAttrs;
    unsigned int                key;

    /* Get the pointer to the object and hwAttrs */
    object = handle->object;
    hwAttrs = handle->hwAttrs;

    /* Disable preemption while checking if the SPI is open. */
    key = Hwi_disable();

    /* Check if the SPI is open already with the base addr. */
    if (object->isOpen == true) {
        Hwi_restore(key);

        Log_warning1("SPI:(%p) already in use.", hwAttrs->baseAddr);

        return (NULL);
    }

    /* Mark the handle as being used */
    object->isOpen = true;
    Hwi_restore(key);

    Assert_isTrue((params->dataSize >= 4) &&
                  (params->dataSize <= 16), NULL);

    /* Initialize the SPI object */
    object->currentTransaction = NULL;
    object->bitRate            = params->bitRate;
    object->dataSize           = params->dataSize;
    object->frameFormat        = params->frameFormat;
    object->mode               = params->mode;
    object->transferMode       = params->transferMode;
    object->transferTimeout    = params->transferTimeout;
    object->returnPartial      = false;
#ifdef SPICC26XXDMA_WAKEUP_ENABLED
    object->wakeupCallbackFxn  = NULL;
#endif

    /* Determine if we need to use an 8-bit or 16-bit framesize for the DMA */
    object->frameSize = (params->dataSize < 9) ? SPICC26XXDMA_8bit : SPICC26XXDMA_16bit;

    Log_print2(Diags_USER2,"SPI:(%p) DMA buffer incrementation size: %s",
                            hwAttrs->baseAddr,
                           (object->frameSize) ? (UArg)"16-bit" : (UArg)"8-bit");

    /* Register power dependency - i.e. power up and enable clock for SPI. */
    Power_setDependency(hwAttrs->powerMngrId);

    /* Configure the hardware module */
    SPICC26XXDMA_initHw(handle);

    /* CSN is initialized using hwAttrs initially, but can be re-configured later */
    object->csnPin = hwAttrs->csnPin;

    /* Configure IOs after hardware has been initialized so that IOs aren't */
    /* toggled unnecessary and make sure it was successful */
    if (!SPICC26XXDMA_initIO(handle)) {
        /* Trying to use SPI driver when some other driver or application
        *  has already allocated these pins, error! */
        Log_warning0("Could not allocate SPI pins, already in use.");

        /* Release power dependency - i.e. potentially power down serial domain. */
        Power_releaseDependency(hwAttrs->powerMngrId);

        /* Mark the module as available */
        key = Hwi_disable();
        object->isOpen = false;
        Hwi_restore(key);

        /* Signal back to application that SPI driver was not succesfully opened */
        return (NULL);
    }

    /* Create the Hwi for this SPI peripheral. */
    Hwi_Params_init(&paramsUnion.hwiParams);
    paramsUnion.hwiParams.arg = (UArg) handle;
    paramsUnion.hwiParams.priority = hwAttrs->intPriority;
    Hwi_construct(&(object->hwi), (int) hwAttrs->intNum, SPICC26XXDMA_hwiFxn, &paramsUnion.hwiParams, NULL);

    /* Create Swi object for this SPI peripheral */
    Swi_Params_init(&paramsUnion.swiParams);
    paramsUnion.swiParams.arg0 = (UArg)handle;
    paramsUnion.swiParams.priority = hwAttrs->swiPriority;
    Swi_construct(&(object->swi), SPICC26XXDMA_swiFxn, &(paramsUnion.swiParams), NULL);


    /* Check the transfer mode */
    if (object->transferMode == SPI_MODE_BLOCKING) {
        Log_print1(Diags_USER2, "SPI DMA:(%p) in SPI_MODE_BLOCKING mode",
                                 hwAttrs->baseAddr);

        /* Create a semaphore to block task execution for the duration of the
         * SPI transfer */
        Semaphore_Params_init(&paramsUnion.semParams);
        paramsUnion.semParams.mode = Semaphore_Mode_BINARY;
        Semaphore_construct(&(object->transferComplete), 0, &paramsUnion.semParams);

        /* Store internal callback function */
        object->transferCallbackFxn = SPICC26XXDMA_transferCallback;
    }
    else {
        Log_print1(Diags_USER2, "SPI DMA:(%p) in SPI_MODE_CALLBACK mode", hwAttrs->baseAddr);

        /* Check to see if a callback function was defined for async mode */
        Assert_isTrue(params->transferCallbackFxn != NULL, NULL);

        /* Save the callback function pointer */
        object->transferCallbackFxn = params->transferCallbackFxn;
    }

    /* Declare the dependency on the UDMA driver */
    object->udmaHandle = UDMACC26XX_open();

    /* Configure PIN driver for CSN callback in optional RETURN_PARTIAL slave mode */
    /* and/or optional wake up on CSN assert slave mode */
    if (object->mode == SPI_SLAVE) {
        PIN_registerIntCb(object->pinHandle, SPICC26XXDMA_csnCallback);
        PIN_setUserArg(object->pinHandle, (UArg) handle);
    }

    Log_print1(Diags_USER1, "SPI:(%p) opened", hwAttrs->baseAddr);

    /* Register notification functions */
#ifdef SPICC26XXDMA_WAKEUP_ENABLED
    Power_registerNotify(&object->spiPreObj, PowerCC26XX_ENTERING_STANDBY, (Fxn)spiPreNotify, (UInt32)handle);
#endif
    Power_registerNotify(&object->spiPostObj, PowerCC26XX_AWAKE_STANDBY, (Fxn)spiPostNotify, (UInt32)handle);

    return (handle);
}

/*!
 *  @brief  Function for transferring using the SPI interface.
 *
 *  The function will enable the SPI and UDMA modules and disallow
 *  the device from going into standby.
 *
 *  In ::SPI_MODE_BLOCKING, SPI_transfer will block task execution until the transfer
 *  has ended.
 *
 *  In ::SPI_MODE_CALLBACK, SPI_transfer does not block task execution, but calls a
 *  callback function specified by transferCallback when the transfer has ended.
 *
 *  @pre    SPICC26XXDMA_open() has to be called first.
 *          Calling context: Hwi and Swi (only if using ::SPI_MODE_CALLBACK), Task
 *
 *  @param  handle A SPI handle returned from SPICC26XXDMA_open()
 *
 *  @param  *transaction Pointer to transaction struct
 *
 *  @return True if transfer is successful and false if not
 *
 *  @sa     SPICC26XXDMA_open(), SPICC26XXDMA_transferCancel()
 */
bool SPICC26XXDMA_transfer(SPI_Handle handle, SPI_Transaction *transaction)
{
    unsigned int                key;
    SPICC26XXDMA_Object         *object;
    SPICC26XXDMA_HWAttrs const  *hwAttrs;

    /* Get the pointer to the object and hwAttr*/
    object = handle->object;
    hwAttrs = handle->hwAttrs;

    /* This is a limitation by the uDMA controller */
    Assert_isTrue(transaction->count <= 1024, NULL);

    if (transaction->count == 0) {
        return (false);
    }

    /* Make sure that the buffers are aligned properly */
    if (object->frameSize == SPICC26XXDMA_16bit) {
        Assert_isTrue(!((unsigned long)transaction->txBuf & 0x1), NULL);
        Assert_isTrue(!((unsigned long)transaction->rxBuf & 0x1), NULL);
    }

    /* Disable preemption while checking if a transfer is in progress */
    key = Hwi_disable();
    if (object->currentTransaction) {
        Hwi_restore(key);

        Log_error1("SPI:(%p) transaction still in progress",
                ((SPICC26XXDMA_HWAttrs const *)(handle->hwAttrs))->baseAddr);

        /* Flag that the transfer failed to start */
        transaction->status = SPI_TRANSFER_FAILED;

        /* Transfer is in progress */
        return (false);
    }

    /* Make sure to flag that a transaction is now active */
    transaction->status = SPI_TRANSFER_STARTED;
    object->currentTransaction = transaction;

    Hwi_restore(key);

    /* In slave mode, optionally enable callback on CSN de-assert */
    if (object->returnPartial) {
        PIN_setInterrupt(object->pinHandle, object->csnPin | PIN_IRQ_POSEDGE);
    }

    /* Set constraints to guarantee transaction */
    threadSafeConstraintSet((uint32_t)(transaction->txBuf), object);

    /* Enable the SPI module */
    SSIEnable(hwAttrs->baseAddr);

    /* Setup DMA transfer. */
    SPICC26XXDMA_configDMA(handle, transaction);

    /* Enable the RX overrun interrupt in the SSI module */
    SSIIntEnable(hwAttrs->baseAddr, SSI_RXOR);

    if (object->transferMode == SPI_MODE_BLOCKING) {
        Log_print1(Diags_USER1, "SPI:(%p) transfer pending on transferComplete "
                                "semaphore",
                ((SPICC26XXDMA_HWAttrs const *)(handle->hwAttrs))->baseAddr);

        if (!Semaphore_pend(Semaphore_handle(&(object->transferComplete)), object->transferTimeout)) {
            /* Mark the transfer as failed. Otherwise SPICC26XXDMA_transferCancel will set it to canceled. */
            if (object->currentTransaction->status == SPI_TRANSFER_STARTED) {
                object->currentTransaction->status = SPI_TRANSFER_FAILED;
            }
            /* Cancel the transfer, if we experience a timeout */
            SPICC26XXDMA_transferCancel(handle);
            /*
             * SPICC26XXDMA_transferCancel peforms a callback which posts a
             * transferComplete semaphore. This call consumes this extra post.
             */
            Semaphore_pend(Semaphore_handle(&(object->transferComplete)),
                           BIOS_NO_WAIT);
            return (false);
        }
    }
    return (true);
}

/*!
 *  @brief Function that cancels a SPI transfer. Will disable SPI and UDMA modules
 *         and allow standby.
 *
 *  @pre    SPICC26XXDMA_open() has to be called first.
 *          Calling context: Task
 *
 *  @param handle         The SPI_Handle for ongoing transaction.
 */
void SPICC26XXDMA_transferCancel(SPI_Handle handle) {
    SPICC26XXDMA_Object         *object;
    SPICC26XXDMA_HWAttrs const  *hwAttrs;
    SPI_Transaction             *transaction;
    volatile tDMAControlTable   *dmaControlTableEntry;

    /* Get the pointer to the object and hwAttrs */
    object = handle->object;
    hwAttrs = handle->hwAttrs;

    /* Disable interrupts to ensure that this function is not interrupted. The calls to SPICC26XXDMA_transferCancel
     * within the driver are safe. However, this function may be called by the application in callback mode.
     * Hence, it might be pre-empted by the SPI hwi and swi or the PIN hwi and swi.
     */
    uint32_t key = Hwi_disable();

    /* Check if there is an active transaction */
    if(!(object->currentTransaction)) {
        Log_print1(Diags_USER1,"SPI:(%p) No transaction to cancel.",
                            hwAttrs->baseAddr);
        Hwi_restore(key);
        return;
    }
    transaction = object->currentTransaction;

    Hwi_restore(key);

    /* Disable the SPI module */
    SSIDisable(hwAttrs->baseAddr);

    /* Disable SPI TX/RX DMA and clear DMA done interrupt just in case it finished */
    SSIDMADisable(hwAttrs->baseAddr, SSI_DMA_TX | SSI_DMA_RX);
    UDMACC26XX_clearInterrupt(object->udmaHandle, (hwAttrs->rxChannelBitMask) | (hwAttrs->txChannelBitMask));

    /* Disable and clear any pending interrupts */
    SSIIntDisable(hwAttrs->baseAddr, SSI_RXOR);
    SSIIntClear(hwAttrs->baseAddr, SSI_RXOR);

    /* Flush the FIFOs and re-initialize module */
    SPICC26XXDMA_flushFifos(handle);

    /* Release constraint since transaction is done */
    threadSafeConstraintRelease((uint32_t)(transaction->txBuf), object);

    /* Mark the transaction as failed if we didn't end up here due to a CSN deassertion */
    if (transaction->status == SPI_TRANSFER_STARTED) {
        transaction->status = SPI_TRANSFER_CANCELED;
    }

    /* Disable the UDMA channels */
    UDMACC26XX_channelDisable(object->udmaHandle, (hwAttrs->rxChannelBitMask) | (hwAttrs->txChannelBitMask));

    /* Update the SPI_Transaction.count parameter */
    /* rxChannel always finishes after txChannel so remaining bytes of the rxChannel is used to update count */
    dmaControlTableEntry = (hwAttrs->baseAddr == SSI0_BASE ? &dmaSpi0RxControlTableEntry : &dmaSpi1RxControlTableEntry);
    transaction->count -= UDMACC26XX_GET_TRANSFER_SIZE(dmaControlTableEntry->ui32Control);

    /* Post SWI to handle remaining clean up and invocation of callback */
    Swi_post(Swi_handle(&(object->swi)));

    Log_print2(Diags_USER1,"SPI:(%p) DMA transaction: %p cancelled",
                            hwAttrs->baseAddr, (UArg)transaction);

    return;
}

/*
 *  ======== SPICC26XXDMA_transferCallback ========
 *  Callback function for when the SPI is in SPI_MODE_BLOCKING
 *
 *  @pre    Function assumes that the handle is not NULL
 */
static void SPICC26XXDMA_transferCallback(SPI_Handle handle, SPI_Transaction *msg)
{
    SPICC26XXDMA_Object         *object;

    Log_print1(Diags_USER1, "SPI DMA:(%p) posting transferComplete semaphore",
                ((SPICC26XXDMA_HWAttrs const *)(handle->hwAttrs))->baseAddr);

    /* Get the pointer to the object */
    object = handle->object;

    /* Post the semaphore */
    Semaphore_post(Semaphore_handle(&(object->transferComplete)));
}

/*
 *  ======== SPICC26XXDMA_csnDeassertCallback ========
 *  Slave mode optional callback function for when the CSN is deasserted
 *
 *  @pre    Function assumes that the handle is not NULL
 */
static void SPICC26XXDMA_csnCallback(PIN_Handle handle, PIN_Id pinId)
{
    SPICC26XXDMA_Object        *object;
    SPI_Handle              spiHandle;
    PIN_Config              csnConfig;

    /* Get the pointer to the SPI object */
    spiHandle = (SPI_Handle) PIN_getUserArg(handle);
    object    = spiHandle->object;

    /* Get current CSN config */
    csnConfig = PIN_getConfig(object->csnPin);

    /* Disable all interrupts */
    PIN_setInterrupt(handle, object->csnPin);

    /* Cancel transfer if POSEDGE interrupt */
    /* TODO: Consider doing this in a SWI */
    if ((csnConfig & PIN_IRQ_POSEDGE) == PIN_IRQ_POSEDGE) {
        /* Indicate why the transaction completed */
        if ((object->currentTransaction != NULL) && (object->currentTransaction->status == SPI_TRANSFER_STARTED)) {
            object->currentTransaction->status = SPI_TRANSFER_CSN_DEASSERT;
        }
        /* Cancel the current transaction */
        SPICC26XXDMA_transferCancel(spiHandle);
    }
}

/*
*  ======== SPICC26XXDMA_hwInit ========
*  This functions initializes the SPI hardware module.
*
*  @pre    Function assumes that the SPI handle is pointing to a hardware
*          module which has already been opened.
*/
static void SPICC26XXDMA_initHw(SPI_Handle handle) {
    SPICC26XXDMA_Object        *object;
    SPICC26XXDMA_HWAttrs const *hwAttrs;
    Types_FreqHz                freq;

    /* Get the pointer to the object and hwAttrs */
    object = handle->object;
    hwAttrs = handle->hwAttrs;

    /* Disable SSI operation */
    SSIDisable(hwAttrs->baseAddr);

    /* Disable SPI module interrupts */
    SSIIntDisable(hwAttrs->baseAddr, SSI_RXOR | SSI_RXFF | SSI_RXTO | SSI_TXFF);
    SSIIntClear(hwAttrs->baseAddr, SSI_RXOR | SSI_RXTO);

    /* Set the SPI configuration */
    BIOS_getCpuFreq(&freq);
    SSIConfigSetExpClk(hwAttrs->baseAddr, freq.lo, frameFormat[object->frameFormat],
                       mode[object->mode], object->bitRate, object->dataSize);

    /* Print the configuration */
    Log_print3(Diags_USER1, "SPI:(%p) CPU freq: %d; SPI freq to %d",
               hwAttrs->baseAddr, freq.lo, object->bitRate);
}

/*
*  ======== SPICC26XXDMA_hwInit ========
*  This functions initializes the SPI IOs.
*
*  @pre    Function assumes that the SPI handle is pointing to a hardware
*          module which has already been opened.
*/
static bool SPICC26XXDMA_initIO(SPI_Handle handle) {
    SPICC26XXDMA_Object         *object;
    SPICC26XXDMA_HWAttrs const  *hwAttrs;
    PIN_Config                  spiPinTable[5];
    uint32_t i = 0;

    /* Get the pointer to the object and hwAttrs */
    object = handle->object;
    hwAttrs = handle->hwAttrs;

    /* Configure IOs */
    /* Build local list of pins, allocate through PIN driver and map HW ports */
    if (object->mode == SPI_SLAVE) {
      /* Configure IOs for slave mode */
      spiPinTable[i++] = hwAttrs->mosiPin | PIN_INPUT_EN;
      spiPinTable[i++] = hwAttrs->misoPin | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_INPUT_DIS | PIN_DRVSTR_MED;
      spiPinTable[i++] = hwAttrs->clkPin  | PIN_INPUT_EN;
      spiPinTable[i++] = object->csnPin   | PIN_INPUT_EN | PIN_PULLUP;
    }
    else {
      /* Configure IOs for master mode */
      spiPinTable[i++] = hwAttrs->mosiPin | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_INPUT_DIS | PIN_DRVSTR_MED;
      spiPinTable[i++] = hwAttrs->misoPin | PIN_INPUT_EN | PIN_PULLDOWN;

      /* Output low signal on SCLK until SPI module drives signal if clock polarity is configured to '0' */
      /* Output high signal on SCLK until SPI module drives signal if clock polarity is configured to '1' */
      if (object->frameFormat == SPI_POL0_PHA0 || object->frameFormat == SPI_POL0_PHA1) {
          spiPinTable[i++] = hwAttrs->clkPin | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_INPUT_DIS | PIN_DRVSTR_MED;
      }
      else {
          spiPinTable[i++] = hwAttrs->clkPin | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_INPUT_DIS | PIN_DRVSTR_MED;
      }

      /* If CSN isn't SW controlled, drive it high until SPI module drives signal to avoid glitches */
      if(object->csnPin != PIN_UNASSIGNED) {
          spiPinTable[i++] = object->csnPin | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL | PIN_INPUT_DIS | PIN_DRVSTR_MED;
      }
    }
    spiPinTable[i++] = PIN_TERMINATE;

    /* Open and assign pins through pin driver */
    if (!(object->pinHandle = PIN_open(&(object->pinState), spiPinTable))) {
        return false;
    }

    /* Set IO muxing for the SPI pins */
    if (mode[object->mode] == SSI_MODE_SLAVE) {
        /* Configure IOs for slave mode */
        PINCC26XX_setMux(object->pinHandle, hwAttrs->mosiPin, (hwAttrs->baseAddr == SSI0_BASE ? IOC_PORT_MCU_SSI0_RX  : IOC_PORT_MCU_SSI1_RX));
        PINCC26XX_setMux(object->pinHandle, hwAttrs->misoPin, (hwAttrs->baseAddr == SSI0_BASE ? IOC_PORT_MCU_SSI0_TX  : IOC_PORT_MCU_SSI1_TX));
        PINCC26XX_setMux(object->pinHandle, hwAttrs->clkPin,  (hwAttrs->baseAddr == SSI0_BASE ? IOC_PORT_MCU_SSI0_CLK : IOC_PORT_MCU_SSI1_CLK));
        PINCC26XX_setMux(object->pinHandle, object->csnPin,   (hwAttrs->baseAddr == SSI0_BASE ? IOC_PORT_MCU_SSI0_FSS : IOC_PORT_MCU_SSI1_FSS));
    }
    else {
        /* Configure IOs for master mode */
        PINCC26XX_setMux(object->pinHandle, hwAttrs->mosiPin, (hwAttrs->baseAddr == SSI0_BASE ? IOC_PORT_MCU_SSI0_TX  : IOC_PORT_MCU_SSI1_TX));
        PINCC26XX_setMux(object->pinHandle, hwAttrs->misoPin, (hwAttrs->baseAddr == SSI0_BASE ? IOC_PORT_MCU_SSI0_RX  : IOC_PORT_MCU_SSI1_RX));
        PINCC26XX_setMux(object->pinHandle, hwAttrs->clkPin,  (hwAttrs->baseAddr == SSI0_BASE ? IOC_PORT_MCU_SSI0_CLK : IOC_PORT_MCU_SSI1_CLK));
        if(object->csnPin != PIN_UNASSIGNED) {
            PINCC26XX_setMux(object->pinHandle, object->csnPin, (hwAttrs->baseAddr == SSI0_BASE ? IOC_PORT_MCU_SSI0_FSS  : IOC_PORT_MCU_SSI1_FSS));
        }
    }

    return true;
}

#ifdef SPICC26XXDMA_WAKEUP_ENABLED
/*
 *  ======== spiPreNotify ========
 *  This functions is called to notify the SPI driver of a imminent transition
 *  in to sleep mode.
 *
 *  @pre    Function assumes that the SPI handle (clientArg) is pointing to a
 *          hardware module which has already been opened.
 */
static int spiPreNotify(unsigned int eventType, uintptr_t eventArg, uintptr_t clientArg)
{
    SPICC26XXDMA_Object *object;
    object = ((SPI_Handle) clientArg)->object;

    /* In slave mode, optionally enable wakeup on CSN assert */
    if (object->wakeupCallbackFxn) {
        PIN_setInterrupt(object->pinHandle, object->csnPin | PIN_IRQ_NEGEDGE);
    }
    return Power_NOTIFYDONE;
}
#endif

/*
 *  ======== spiPostNotify ========
 *  This functions is called to notify the SPI driver of an ongoing transition
 *  out of sleep mode.
 *
 *  @pre    Function assumes that the SPI handle (clientArg) is pointing to a
 *          hardware module which has already been opened.
 */
static int spiPostNotify(unsigned int eventType, uintptr_t eventArg, uintptr_t clientArg)
{
    SPI_Handle spiHandle;
#ifdef SPICC26XXDMA_WAKEUP_ENABLED
    SPICC26XXDMA_Object *object;
#endif

    /* Get the pointers to SPI objects */
    spiHandle = (SPI_Handle) clientArg;
#ifdef SPICC26XXDMA_WAKEUP_ENABLED
    object = spiHandle->object;
#endif

    /* Reconfigure the hardware when returning from standby */
    SPICC26XXDMA_initHw(spiHandle);

#ifdef SPICC26XXDMA_WAKEUP_ENABLED
    /* check if waken up by CSN */
    /* PIN_hwi() is called after this in whick GPIO_O_EVFLAGS31_0 is cleared */
    /* Therefore must check it here, not in SPICC26XXDMA_csnCallback */
    if (HWREG(GPIO_BASE + GPIO_O_EVFLAGS31_0) == (0x1 << object->csnPin)) {
        /* Call user defined wake up callback */
        if (object->wakeupCallbackFxn) {
            object->wakeupCallbackFxn(spiHandle);
        }
    }
#endif

    return Power_NOTIFYDONE;
}
