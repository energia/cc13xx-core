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
 *  ======== NVSCC26XX.c ========
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>  /* for string support */
#include <stdlib.h>

#include <xdc/runtime/Assert.h>
#include <xdc/runtime/Diags.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/Log.h>

#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Semaphore.h>
// TODO: Why not hal/Hwi.h?
#include <ti/sysbios/family/arm/m3/Hwi.h>

#include <ti/drivers/NVS.h>
#include <ti/drivers/nvs/NVSCC26XX.h>

#ifdef DEVICE_FAMILY
    #undef DEVICE_FAMILY_PATH
    #define DEVICE_FAMILY_PATH(x) <ti/devices/DEVICE_FAMILY/x>
#else
    #error "You must define DEVICE_FAMILY at the project level as one of cc26x0, cc26x0r2, cc13x0, etc."
#endif

#include DEVICE_FAMILY_PATH(driverlib/flash.h)
#include DEVICE_FAMILY_PATH(driverlib/vims.h)

/* NVSCC26XX functions */
void        NVSCC26XX_close(NVS_Handle handle);
int         NVSCC26XX_control(NVS_Handle handle, unsigned int cmd,
                                uintptr_t arg);
void        NVSCC26XX_exit(NVS_Handle handle);
int         NVSCC26XX_getAttrs(NVS_Handle handle, NVS_Attrs *attrs);
void        NVSCC26XX_init(NVS_Handle handle);
NVS_Handle  NVSCC26XX_open(NVS_Handle handle, NVS_Params *params);
int         NVSCC26XX_read(NVS_Handle handle, size_t offset, void *buffer,
                             size_t bufferSize);
int         NVSCC26XX_write(NVS_Handle handle, size_t offset, void *buffer,
                              size_t bufferSize, unsigned int flags);

/* NVS function table for NVSCC26XX implementation */
const NVS_FxnTable NVSCC26XX_fxnTable = {
    NVSCC26XX_close,
    NVSCC26XX_control,
    NVSCC26XX_exit,
    NVSCC26XX_getAttrs,
    NVSCC26XX_init,
    NVSCC26XX_open,
    NVSCC26XX_read,
    NVSCC26XX_write
};

/*
 *  Semaphore to synchronize access to flash block.
 */
static Semaphore_Struct  writeSem;

static bool isInitialized = false;

/*
 *  ======== disableFlashCache ========
 *  When updating the Flash, the VIMS (Vesatile Instruction Memory System)
 *  mode must be set to GPRAM or OFF, before programming, and both VIMS
 *  flash line buffers must be set to disabled.
 */
static uint8_t disableFlashCache(void)
{
    uint8_t mode = VIMSModeGet(VIMS_BASE);

    if (mode != VIMS_MODE_DISABLED) {
        VIMSModeSet(VIMS_BASE, VIMS_MODE_DISABLED);
        while (VIMSModeGet(VIMS_BASE) != VIMS_MODE_DISABLED);
    }

    return (mode);
}

/*
 *  ======== enableFlashCache ========
 */
static void enableFlashCache(uint8_t mode)
{
    if (mode != VIMS_MODE_DISABLED) {
        VIMSModeSet(VIMS_BASE, VIMS_MODE_ENABLED);
    }
}

/*
 *  ======== NVSCC26XX_close ========
 */
void NVSCC26XX_close(NVS_Handle handle)
{
}

/*
 *  ======== NVSCC26XX_control ========
 */
int NVSCC26XX_control(NVS_Handle handle, unsigned int cmd, uintptr_t arg)
{
    NVSCC26XX_HWAttrs *hwAttrs = (NVSCC26XX_HWAttrs *)(handle->hwAttrs);
    NVSCC26XX_CmdSetCopyBlockArgs *cmdArgs = (NVSCC26XX_CmdSetCopyBlockArgs *)arg;
    uint8_t *copyBlock = (uint8_t *)(cmdArgs->copyBlock);

    if (cmd == NVSCC26XX_CMD_SET_COPYBLOCK) {
        if ((copyBlock == NULL) || ((uint32_t)copyBlock & 0x3)) {
            return (NVSCC26XX_STATUS_ECOPYBLOCK);
        }

        hwAttrs->copyBlock = cmdArgs->copyBlock;
        hwAttrs->isRam = cmdArgs->isRam;

        if (!hwAttrs->isRam) {
            /* If copy block is in flash, check that it is page aligned */
            if ((uint32_t)(hwAttrs->copyBlock) & (FlashSectorSizeGet() - 1)) {

                Log_warning1("NVS:(%p) Copy block not aligned on page boundary.",
                        (IArg)(hwAttrs->block));
                return (NVSCC26XX_STATUS_ECOPYBLOCK);
            }
        }

        return (NVS_STATUS_SUCCESS);
    }

    return (NVS_STATUS_UNDEFINEDCMD);
}

/*
 *  ======== NVSCC26XX_exit ========
 */
void NVSCC26XX_exit(NVS_Handle handle)
{
}

/*
 *  ======== NVSCC26XX_getAttrs ========
 */
int NVSCC26XX_getAttrs(NVS_Handle handle, NVS_Attrs *attrs)
{
    NVSCC26XX_HWAttrs const  *hwAttrs = handle->hwAttrs;

    /* FlashSectorSizeGet() returns the size of a flash sector in bytes. */
    attrs->pageSize   = FlashSectorSizeGet();
    attrs->blockSize  = hwAttrs->blockSize;

    return (NVS_SOK);
}

/*
 *  ======== NVSCC26XX_init ========
 */
void NVSCC26XX_init(NVS_Handle handle)
{
    if (!isInitialized) {
        Semaphore_construct(&writeSem, 1, NULL);
        isInitialized = true;
    }
}

/*
 *  ======== NVSCC26XX_open =======
 */
NVS_Handle NVSCC26XX_open(NVS_Handle handle, NVS_Params *params)
{
    NVSCC26XX_Object         *object = handle->object;
    NVSCC26XX_HWAttrs const  *hwAttrs = handle->hwAttrs;
    int                       status = NVS_SOK;

    Semaphore_pend(Semaphore_handle(&writeSem), BIOS_WAIT_FOREVER);

    if (object->opened == true) {
        Semaphore_post(Semaphore_handle(&writeSem));

        Log_warning1("NVS:(%p) already in use.", (IArg)(hwAttrs->block));
        return (NULL);
    }

    /* The block must be aligned on a flaah page boundary */
    if ((uint32_t)(hwAttrs->block) & (FlashSectorSizeGet() - 1)) {
        Semaphore_post(Semaphore_handle(&writeSem));

        Log_warning1("NVS:(%p) block not aligned on flash page boundary.",
                (IArg)(hwAttrs->block));
        return (NULL);
    }

    /* The block cannot be larger than a flash page */
    if ((uint32_t)(hwAttrs->blockSize) > FlashSectorSizeGet()) {
        Semaphore_post(Semaphore_handle(&writeSem));

        Log_warning1("NVS:(%p) blockSize must not be greater than page size.",
                (IArg)(hwAttrs->block));
        return (NULL);
    }

    /* Flash copy block must be aligned on a flaah page boundary */
    if (hwAttrs->copyBlock && !(hwAttrs->isRam) &&
            ((uint32_t)(hwAttrs->copyBlock) & (FlashSectorSizeGet() - 1))) {
        Semaphore_post(Semaphore_handle(&writeSem));

        Log_warning1("NVS:(%p) Flash copyBlock not page boundary aligned.",
                (IArg)(hwAttrs->block));
        return (NULL);
    }

    /* Ram copy block must be 4-byte aligned */
    if ((uint32_t)(hwAttrs->copyBlock) & 0x3) {
        Semaphore_post(Semaphore_handle(&writeSem));

        Log_warning1("NVS:(%p) copyBlock not 4-byte aligned.",
                (IArg)(hwAttrs->block));
        return (NULL);
    }

    if (params->eraseOnOpen == true) {
        status = NVSCC26XX_write(handle, 0, NULL, 0, 0);

        if  (status != NVS_SOK) {
            Log_warning1("NVS:(%p) copyBlock not 4-byte aligned.",
                    (IArg)(hwAttrs->block));
        }
    }

    if (status == NVS_SOK) {
        object->opened = true;
    }

    Semaphore_post(Semaphore_handle(&writeSem));

    return (handle);
}

/*
 *  ======== NVSCC26XX_read =======
 */
int NVSCC26XX_read(NVS_Handle handle, size_t offset, void *buffer,
        size_t bufferSize)
{
    NVSCC26XX_HWAttrs const  *hwAttrs = handle->hwAttrs;
    int retval = NVS_SOK;

    /* Validate offset and bufferSize */
    if (offset + bufferSize > hwAttrs->blockSize) {
        return (NVS_EOFFSET);
    }

    /*
     *  Get exclusive access to the block.  We don't want someone
     *  else to erase the block while we are reading it.
     */
    Semaphore_pend(Semaphore_handle(&writeSem), BIOS_WAIT_FOREVER);

    memcpy(buffer, (Char *)(hwAttrs->block) + offset, bufferSize);

    Semaphore_post(Semaphore_handle(&writeSem));

    return (retval);
}

/*
 *  ======== NVSCC26XX_write =======
 */
int NVSCC26XX_write(NVS_Handle handle, size_t offset, void *buffer,
                      size_t bufferSize, unsigned int flags)
{
    NVSCC26XX_HWAttrs const  *hwAttrs = handle->hwAttrs;
    unsigned int size;
    uint32_t status = 0;
    int i;
    uint8_t mode;
    uint8_t *srcBuf, *dstBuf;
    int retval = NVS_SOK;

    /* Buffer to copy into flash must be 4-byte aligned */
    if (((uint32_t)buffer & 0x3) || (bufferSize & 0x3)) {
        Log_warning1("NVS:(%p) Buffer and buffer size must be 4-byte aligned.",
                (IArg)(hwAttrs->block));
        return (NVS_EALIGN);
    }

    /* Check if offset is not a multiple of 4 */
    if (offset & 0x3) {
        Log_warning1("NVS:(%p) offset size must be 4-byte aligned.",
                (IArg)(hwAttrs->block));
        return (NVS_EALIGN);
    }

    /* Validate offset and bufferSize */
    if (offset + bufferSize > hwAttrs->blockSize) {
        return (NVS_EOFFSET);
    }

    /* Get exclusive access to the Flash block */
    Semaphore_pend(Semaphore_handle(&writeSem), BIOS_WAIT_FOREVER);

    /*
     *  A NULL buffer signifies that we are just erasing the
     *  flash block.
     */
    if (buffer == NULL) {
        mode = disableFlashCache();
        status = FlashSectorErase((uint32_t)hwAttrs->block);
        enableFlashCache(mode);

        Semaphore_post(Semaphore_handle(&writeSem));

        if (status != FAPI_STATUS_SUCCESS) {
            Log_warning1("NVS:(%p) FlashSectorErase() failed.",
                    (IArg)(hwAttrs->block));
            return (NVS_EFAIL);
        }

        return (NVS_SOK);
    }

    /*
     *  If exclusive write, check that the region has not been
     *  written to since the last erase.  (Erasing leaves flash
     *  set to 0xFF)
     */
    if (flags & NVS_WRITE_EXCLUSIVE) {
        dstBuf = (uint8_t *)((uint32_t)(hwAttrs->block) + offset);
        for (i = 0; i < bufferSize; i++) {
            if (dstBuf[i] != 0xFF) {
                Semaphore_post(Semaphore_handle(&writeSem));
                return (NVS_EALREADYWRITTEN);
            }
        }
    }

    /* If erase is set, determine whether to use RAM or the flash copyBlock */
    if (flags & NVS_WRITE_ERASE) {

        /* Must have copy block for erase */
        if (hwAttrs->copyBlock == NULL) {
            Semaphore_post(Semaphore_handle(&writeSem));
            Log_warning1("NVS:(%p) copyBlock must be non-NULL.",
                    (IArg)(hwAttrs->block));
            return (NVS_ECOPYBLOCK);
        }

        srcBuf = (uint8_t *)(hwAttrs->copyBlock);

        if (hwAttrs->isRam) {
            /* Copy flash contents up to the offset into temporary buffer */
            memcpy(srcBuf, hwAttrs->block, offset);

            /* Update the temporary buffer with the data to be written */
            memcpy((void *)((uint32_t)srcBuf + offset), buffer, bufferSize);

            /* Copy remaining flash contents into temporary buffer */
            memcpy(srcBuf + offset + bufferSize,
                    (void *)((uint32_t)hwAttrs->block + offset + bufferSize),
                    hwAttrs->blockSize - bufferSize - offset);
        }
        else {
            /* Erase the flash copy block */
            mode = disableFlashCache();
            status = FlashSectorErase((uint32_t)(hwAttrs->copyBlock));
            if (status != 0) {
                enableFlashCache(mode);
                Semaphore_post(Semaphore_handle(&writeSem));

                Log_warning1("NVS:(%p) FlashSectorErase() failed.",
                        (IArg)(hwAttrs->block));
                return (NVS_EFAIL);
            }

            /*  Copy up to offset */
            status = FlashProgram((uint8_t *)(hwAttrs->block),    /* src  */
                                  (uint32_t)hwAttrs->copyBlock,   /* dst  */
                                  (uint32_t)offset);              /* size */

            /*  Copy buffer */
            status |= FlashProgram((uint8_t *)buffer,
                                  (uint32_t)(hwAttrs->copyBlock) + offset,
                                  (uint32_t)bufferSize);

            /*  Copy after offset + bufferSize */
            status |= FlashProgram(
                (uint8_t *)((uint32_t)hwAttrs->block + offset + bufferSize),
                (uint32_t)(hwAttrs->copyBlock) + offset + bufferSize,
                hwAttrs->blockSize - bufferSize - offset);

            enableFlashCache(mode);

            if (status != 0) {
                Semaphore_post(Semaphore_handle(&writeSem));

                Log_warning1("NVS:(%p) FlashProgram() failed.",
                        (IArg)(hwAttrs->block));
                return (NVS_EFAIL);
            }
        }

        mode = disableFlashCache();
        status = FlashSectorErase((uint32_t)hwAttrs->block);
        enableFlashCache(mode);

        if (status != 0) {
            Semaphore_post(Semaphore_handle(&writeSem));
            Log_warning1("NVS:(%p) FlashSectorErase() failed.",
                    (IArg)(hwAttrs->block));
            return (NVS_EFAIL);
        }
        size = hwAttrs->blockSize;
        dstBuf = hwAttrs->block;
    }
    else {
        srcBuf = buffer;
        size   = bufferSize;
        dstBuf = (uint8_t *)((uint32_t)(hwAttrs->block) + offset);
    }

    mode = disableFlashCache();
    status = FlashProgram((uint8_t*)srcBuf, (uint32_t)dstBuf, size);
    enableFlashCache(mode);

    if (status != 0) {
        retval = NVS_EFAIL;
    }
    else if ((flags & NVS_WRITE_VALIDATE)) {
        /*
         *  Note: This validates the entire block even on erase mode.
         */
        for (i = 0; i < size; i++) {
            if (srcBuf[i] != dstBuf[i]) {
                retval = NVS_EFAIL;
                break;
            }
        }
    }

    Semaphore_post(Semaphore_handle(&writeSem));

    return (retval);
}
