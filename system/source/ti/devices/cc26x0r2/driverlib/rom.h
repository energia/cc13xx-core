/******************************************************************************
*  Filename:       rom.h
*  Revised:        2016-10-06 17:21:09 +0200 (Thu, 06 Oct 2016)
*  Revision:       47343
*
*  Description:    Prototypes for the ROM utility functions.
*
*  Copyright (c) 2015 - 2017, Texas Instruments Incorporated
*  All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions are met:
*
*  1) Redistributions of source code must retain the above copyright notice,
*     this list of conditions and the following disclaimer.
*
*  2) Redistributions in binary form must reproduce the above copyright notice,
*     this list of conditions and the following disclaimer in the documentation
*     and/or other materials provided with the distribution.
*
*  3) Neither the name of the ORGANIZATION nor the names of its contributors may
*     be used to endorse or promote products derived from this software without
*     specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
*  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
*  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
*  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
*  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
*  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
*  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
*  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
*  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
*  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
*  POSSIBILITY OF SUCH DAMAGE.
*
******************************************************************************/

#ifndef __ROM_H__
#define __ROM_H__

//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif

#include "../inc/hw_types.h"

#ifndef __HAPI_H__
#define __HAPI_H__

// Start address of the ROM hard API access table (located after the ROM FW rev field)
#define ROM_HAPI_TABLE_ADDR 0x10000048

// ROM Hard-API function interface types
typedef uint32_t     (* FPTR_CRC32_T)              ( uint8_t*     /* pui8Data        */,\
                                                     uint32_t     /* ui32ByteCount   */,\
                                                     uint32_t     /* ui32RepeatCount */);

typedef uint32_t     (* FPTR_GETFLSIZE_T)          ( void );

typedef uint32_t     (* FPTR_GETCHIPID_T)          ( void );

typedef uint32_t     (* FPTR_RESERVED1_T)          ( uint32_t );

typedef uint32_t     (* FPTR_RESERVED2_T)          ( void );

typedef uint32_t     (* FPTR_RESERVED3_T)          ( uint8_t*                          ,\
                                                     uint32_t                          ,\
                                                     uint32_t                          );
typedef void         (* FPTR_RESETDEV_T)           ( void );

typedef uint32_t     (* FPTR_FLETCHER32_T)         ( uint16_t*    /* pui16Data       */,\
                                                     uint16_t     /* ui16WordCount   */,\
                                                     uint16_t     /* ui16RepeatCount */);

typedef uint32_t     (* FPTR_MINVAL_T)             ( uint32_t*    /* ulpDataBuffer   */,\
                                                     uint32_t     /* ui32DataCount   */);

typedef uint32_t     (* FPTR_MAXVAL_T)             ( uint32_t*    /* pui32DataBuffer */,\
                                                     uint32_t     /* ui32DataCount   */);

typedef uint32_t     (* FPTR_MEANVAL_T)            ( uint32_t*    /* pui32DataBuffer */,\
                                                     uint32_t     /* ui32DataCount   */);

typedef uint32_t     (* FPTR_STDDVAL_T)            ( uint32_t*    /* pui32DataBuffer */,\
                                                     uint32_t     /* ui32DataCount   */);

typedef void         (* FPTR_HFSOURCESAFESWITCH_T) ( void );

typedef void         (* FPTR_RESERVED4_T)          ( uint32_t                          );

typedef void         (* FPTR_RESERVED5_T)          ( uint32_t                          );

typedef void         (* FPTR_COMPAIN_T)            ( uint8_t      /* ut8Signal       */);

typedef void         (* FPTR_COMPAREF_T)           ( uint8_t      /* ut8Signal       */);

typedef void         (* FPTR_ADCCOMPBIN_T)         ( uint8_t      /* ut8Signal       */);

typedef void         (* FPTR_COMPBREF_T)           ( uint8_t      /* ut8Signal       */);

extern uint32_t MemBusWrkAroundHapiProgramFlash(uint8_t *pui8DataBuffer,
                                                uint32_t ui32Address,
                                                uint32_t ui32Count);

extern uint32_t MemBusWrkAroundHapiEraseSector(uint32_t ui32Address);

// ROM Hard-API access table type
typedef struct
{
    FPTR_CRC32_T                    Crc32;
    FPTR_GETFLSIZE_T                FlashGetSize;
    FPTR_GETCHIPID_T                GetChipId;
    FPTR_RESERVED1_T                ReservedLocation1;
    FPTR_RESERVED2_T                ReservedLocation2;
    FPTR_RESERVED3_T                ReservedLocation3;
    FPTR_RESETDEV_T                 ResetDevice;
    FPTR_FLETCHER32_T               Fletcher32;
    FPTR_MINVAL_T                   MinValue;
    FPTR_MAXVAL_T                   MaxValue;
    FPTR_MEANVAL_T                  MeanValue;
    FPTR_STDDVAL_T                  StandDeviationValue;
    FPTR_RESERVED4_T                ReservedLocation4;
    FPTR_RESERVED5_T                ReservedLocation5;
    FPTR_HFSOURCESAFESWITCH_T       HFSourceSafeSwitch;
    FPTR_COMPAIN_T                  SelectCompAInput;
    FPTR_COMPAREF_T                 SelectCompARef;
    FPTR_ADCCOMPBIN_T               SelectADCCompBInput;
    FPTR_COMPBREF_T                 SelectCompBRef;
} HARD_API_T;

// Pointer to the ROM HAPI table
#define P_HARD_API                   ((HARD_API_T*) ROM_HAPI_TABLE_ADDR)

// Add wrapper around the Hapi functions needing the "bus arbitration issue" workaround
extern void SafeHapiVoid( FPTR_VOID_VOID_T fPtr );
extern void SafeHapiAuxAdiSelect( FPTR_VOID_UINT8_T fPtr, uint8_t ut8Signal );

#define HapiCrc32(a,b,c)             P_HARD_API->Crc32(a,b,c)
#define HapiGetFlashSize()           P_HARD_API->FlashGetSize()
#define HapiGetChipId()              P_HARD_API->GetChipId()
#define HapiSectorErase(a)           MemBusWrkAroundHapiEraseSector(a)
#define HapiProgramFlash(a,b,c)      MemBusWrkAroundHapiProgramFlash(a,b,c)
#define HapiResetDevice()            P_HARD_API->ResetDevice()
#define HapiFletcher32(a,b,c)        P_HARD_API->Fletcher32(a,b,c)
#define HapiMinValue(a,b)            P_HARD_API->MinValue(a,b)
#define HapiMaxValue(a,b)            P_HARD_API->MaxValue(a,b)
#define HapiMeanValue(a,b)           P_HARD_API->MeanValue(a,b)
#define HapiStandDeviationValue(a,b) P_HARD_API->StandDeviationValue(a,b)
#define HapiHFSourceSafeSwitch()     SafeHapiVoid( P_HARD_API->HFSourceSafeSwitch )
#define HapiSelectCompAInput(a)      SafeHapiAuxAdiSelect( P_HARD_API->SelectCompAInput   , a )
#define HapiSelectCompARef(a)        SafeHapiAuxAdiSelect( P_HARD_API->SelectCompARef     , a )
#define HapiSelectADCCompBInput(a)   SafeHapiAuxAdiSelect( P_HARD_API->SelectADCCompBInput, a )
#define HapiSelectCompBRef(a)        SafeHapiAuxAdiSelect( P_HARD_API->SelectCompBRef     , a )

// Defines for input parameter to the HapiSelectCompAInput function.
// The define values can not be changed!
#define COMPA_IN_NC            0x00
#define COMPA_IN_AUXIO7        0x09
#define COMPA_IN_AUXIO6        0x0A
#define COMPA_IN_AUXIO5        0x0B
#define COMPA_IN_AUXIO4        0x0C
#define COMPA_IN_AUXIO3        0x0D
#define COMPA_IN_AUXIO2        0x0E
#define COMPA_IN_AUXIO1        0x0F
#define COMPA_IN_AUXIO0        0x10

// Defines for input parameter to the HapiSelectCompARef function.
// The define values can not be changed!
#define COMPA_REF_NC           0x00
#define COMPA_REF_DCOUPL       0x01
#define COMPA_REF_VSS          0x02
#define COMPA_REF_VDDS         0x03
#define COMPA_REF_ADCVREFP     0x04
#define COMPA_REF_AUXIO7       0x09
#define COMPA_REF_AUXIO6       0x0A
#define COMPA_REF_AUXIO5       0x0B
#define COMPA_REF_AUXIO4       0x0C
#define COMPA_REF_AUXIO3       0x0D
#define COMPA_REF_AUXIO2       0x0E
#define COMPA_REF_AUXIO1       0x0F
#define COMPA_REF_AUXIO0       0x10

// Defines for input parameter to the HapiSelectADCCompBInput function.
// The define values can not be changed!
#define ADC_COMPB_IN_NC        0x00
#define ADC_COMPB_IN_DCOUPL    0x03
#define ADC_COMPB_IN_VSS       0x04
#define ADC_COMPB_IN_VDDS      0x05
#define ADC_COMPB_IN_AUXIO7    0x09
#define ADC_COMPB_IN_AUXIO6    0x0A
#define ADC_COMPB_IN_AUXIO5    0x0B
#define ADC_COMPB_IN_AUXIO4    0x0C
#define ADC_COMPB_IN_AUXIO3    0x0D
#define ADC_COMPB_IN_AUXIO2    0x0E
#define ADC_COMPB_IN_AUXIO1    0x0F
#define ADC_COMPB_IN_AUXIO0    0x10

// Defines for input parameter to the HapiSelectCompBRef function.
// The define values can not be changed!
#define COMPB_REF_NC           0x00
#define COMPB_REF_DCOUPL       0x01
#define COMPB_REF_VSS          0x02
#define COMPB_REF_VDDS         0x03

#endif // __HAPI_H__

//*****************************************************************************
//
// Pointers to the main API tables.
//
//*****************************************************************************
#define ROM_API_TABLE           ((uint32_t *) 0x10000180)
#define ROM_VERSION             (ROM_API_TABLE[0])


#define ROM_API_AON_EVENT_TABLE  ((uint32_t*) (ROM_API_TABLE[1]))
#define ROM_API_AON_IOC_TABLE    ((uint32_t*) (ROM_API_TABLE[2]))
#define ROM_API_AON_RTC_TABLE    ((uint32_t*) (ROM_API_TABLE[3]))
#define ROM_API_AON_WUC_TABLE    ((uint32_t*) (ROM_API_TABLE[4]))
#define ROM_API_AUX_CTRL_TABLE   ((uint32_t*) (ROM_API_TABLE[5]))
#define ROM_API_AUX_TDC_TABLE    ((uint32_t*) (ROM_API_TABLE[6]))
#define ROM_API_AUX_TIMER_TABLE  ((uint32_t*) (ROM_API_TABLE[7]))
#define ROM_API_AUX_WUC_TABLE    ((uint32_t*) (ROM_API_TABLE[8]))
#define ROM_API_DDI_TABLE        ((uint32_t*) (ROM_API_TABLE[9]))
#define ROM_API_FLASH_TABLE      ((uint32_t*) (ROM_API_TABLE[10]))
#define ROM_API_I2C_TABLE        ((uint32_t*) (ROM_API_TABLE[11]))
#define ROM_API_INTERRUPT_TABLE  ((uint32_t*) (ROM_API_TABLE[12]))
#define ROM_API_IOC_TABLE        ((uint32_t*) (ROM_API_TABLE[13]))
#define ROM_API_PRCM_TABLE       ((uint32_t*) (ROM_API_TABLE[14]))
#define ROM_API_SMPH_TABLE       ((uint32_t*) (ROM_API_TABLE[15]))
#define ROM_API_SSI_TABLE        ((uint32_t*) (ROM_API_TABLE[17]))
#define ROM_API_TIMER_TABLE      ((uint32_t*) (ROM_API_TABLE[18]))
#define ROM_API_TRNG_TABLE       ((uint32_t*) (ROM_API_TABLE[19]))
#define ROM_API_UART_TABLE       ((uint32_t*) (ROM_API_TABLE[20]))
#define ROM_API_UDMA_TABLE       ((uint32_t*) (ROM_API_TABLE[21]))
#define ROM_API_VIMS_TABLE       ((uint32_t*) (ROM_API_TABLE[22]))
#define ROM_API_CRYPTO_TABLE     ((uint32_t*) (ROM_API_TABLE[23]))
#define ROM_API_OSC_TABLE        ((uint32_t*) (ROM_API_TABLE[24]))
#define ROM_API_SYS_CTRL_TABLE   ((uint32_t*) (ROM_API_TABLE[26]))
#define ROM_API_AON_BATMON_TABLE ((uint32_t*) (ROM_API_TABLE[27]))
#define ROM_API_SETUP_ROM_TABLE  ((uint32_t*) (ROM_API_TABLE[28]))

// AON_EVENT FUNCTIONS
#define ROM_AONEventMcuWakeUpSet \
    ((void (*)(uint32_t ui32MCUWUEvent, uint32_t ui32EventSrc)) \
    ROM_API_AON_EVENT_TABLE[0])


// AON_RTC FUNCTIONS
#define ROM_AONRTCCurrentCompareValueGet \
    ((uint32_t (*)(void)) \
    ROM_API_AON_RTC_TABLE[11])

#define ROM_AONRTCCurrent64BitValueGet \
    ((uint64_t (*)(void)) \
    ROM_API_AON_RTC_TABLE[12])


// AUX_WUC FUNCTIONS
#define ROM_AUXWUCClockEnable \
    ((void (*)(uint32_t ui32Clocks)) \
    ROM_API_AUX_WUC_TABLE[0])

#define ROM_AUXWUCPowerCtrl \
    ((void (*)(uint32_t ui32PowerMode)) \
    ROM_API_AUX_WUC_TABLE[3])


// DDI FUNCTIONS
#define ROM_DDI16BitfieldWrite \
    ((void (*)(uint32_t ui32Base, uint32_t ui32Reg, uint32_t ui32Mask, uint32_t ui32Shift, uint16_t ui32Data)) \
    ROM_API_DDI_TABLE[1])

#define ROM_DDI16BitRead \
    ((uint16_t (*)(uint32_t ui32Base, uint32_t ui32Reg, uint32_t ui32Mask)) \
    ROM_API_DDI_TABLE[2])

#define ROM_DDI16BitfieldRead \
    ((uint16_t (*)(uint32_t ui32Base, uint32_t ui32Reg, uint32_t ui32Mask, uint32_t ui32Shift)) \
    ROM_API_DDI_TABLE[3])

#define ROM_DDI32RegWrite \
    ((void (*)(uint32_t ui32Base, uint32_t ui32Reg, uint32_t ui32Val)) \
    ROM_API_DDI_TABLE[4])


// FLASH FUNCTIONS
#define ROM_FlashPowerModeSet \
    ((void (*)(uint32_t ui32PowerMode, uint32_t ui32BankGracePeriode, uint32_t ui32PumpGracePeriode)) \
    ROM_API_FLASH_TABLE[0])

#define ROM_FlashPowerModeGet \
    ((uint32_t (*)(void)) \
    ROM_API_FLASH_TABLE[1])

#define ROM_FlashProtectionSet \
    ((void (*)(uint32_t ui32SectorAddress, uint32_t ui32ProtectMode)) \
    ROM_API_FLASH_TABLE[2])

#define ROM_FlashProtectionGet \
    ((uint32_t (*)(uint32_t ui32SectorAddress)) \
    ROM_API_FLASH_TABLE[3])

#define ROM_FlashProtectionSave \
    ((uint32_t (*)(uint32_t ui32SectorAddress)) \
    ROM_API_FLASH_TABLE[4])

#define ROM_FlashEfuseReadRow \
    ((bool (*)(uint32_t *pui32EfuseData, uint32_t ui32RowAddress)) \
    ROM_API_FLASH_TABLE[8])

#define ROM_FlashDisableSectorsForWrite \
    ((void (*)(void)) \
    ROM_API_FLASH_TABLE[9])


// INTERRUPT FUNCTIONS
#define ROM_IntEnable \
    ((void (*)(uint32_t ui32Interrupt)) \
    ROM_API_INTERRUPT_TABLE[4])

#define ROM_IntDisable \
    ((void (*)(uint32_t ui32Interrupt)) \
    ROM_API_INTERRUPT_TABLE[5])

#define ROM_IntPendClear \
    ((void (*)(uint32_t ui32Interrupt)) \
    ROM_API_INTERRUPT_TABLE[8])


// IOC FUNCTIONS
#define ROM_IOCPortConfigureSet \
    ((void (*)(uint32_t ui32IOId, uint32_t ui32PortId, uint32_t ui32IOConfig)) \
    ROM_API_IOC_TABLE[0])


// PRCM FUNCTIONS
#define ROM_PRCMPowerDomainOn \
    ((void (*)(uint32_t ui32Domains)) \
    ROM_API_PRCM_TABLE[5])

#define ROM_PRCMPowerDomainOff \
    ((void (*)(uint32_t ui32Domains)) \
    ROM_API_PRCM_TABLE[6])

#define ROM_PRCMPeripheralRunEnable \
    ((void (*)(uint32_t ui32Peripheral)) \
    ROM_API_PRCM_TABLE[7])

#define ROM_PRCMPeripheralRunDisable \
    ((void (*)(uint32_t ui32Peripheral)) \
    ROM_API_PRCM_TABLE[8])

#define ROM_PRCMPeripheralSleepEnable \
    ((void (*)(uint32_t ui32Peripheral)) \
    ROM_API_PRCM_TABLE[9])

#define ROM_PRCMPeripheralSleepDisable \
    ((void (*)(uint32_t ui32Peripheral)) \
    ROM_API_PRCM_TABLE[10])

#define ROM_PRCMPeripheralDeepSleepEnable \
    ((void (*)(uint32_t ui32Peripheral)) \
    ROM_API_PRCM_TABLE[11])

#define ROM_PRCMPeripheralDeepSleepDisable \
    ((void (*)(uint32_t ui32Peripheral)) \
    ROM_API_PRCM_TABLE[12])

#define ROM_PRCMPowerDomainStatus \
    ((uint32_t (*)(uint32_t ui32Domains)) \
    ROM_API_PRCM_TABLE[13])

#define ROM_PRCMDeepSleep \
    ((void (*)(void)) \
    ROM_API_PRCM_TABLE[14])


// SMPH FUNCTIONS
#define ROM_SMPHAcquire \
    ((void (*)(uint32_t ui32Semaphore)) \
    ROM_API_SMPH_TABLE[0])


// TRNG FUNCTIONS
#define ROM_TRNGConfigure \
    ((void (*)(uint32_t ui32MinSamplesPerCycle, uint32_t ui32MaxSamplesPerCycle, uint32_t ui32ClocksPerSample)) \
    ROM_API_TRNG_TABLE[0])

#define ROM_TRNGNumberGet \
    ((uint32_t (*)(uint32_t ui32Word)) \
    ROM_API_TRNG_TABLE[1])


// VIMS FUNCTIONS
#define ROM_VIMSModeSet \
    ((void (*)(uint32_t ui32Base, uint32_t ui32Mode)) \
    ROM_API_VIMS_TABLE[1])

#define ROM_VIMSModeGet \
    ((uint32_t (*)(uint32_t ui32Base)) \
    ROM_API_VIMS_TABLE[2])


// CRYPTO FUNCTIONS
#define ROM_CRYPTOAesEcb \
    ((uint32_t (*)(uint32_t *pui32MsgIn, uint32_t *pui32MsgOut, uint32_t ui32KeyLocation, bool bEncrypt, bool bIntEnable)) \
    ROM_API_CRYPTO_TABLE[0])

#define ROM_CRYPTOAesEcbStatus \
    ((uint32_t (*)(void)) \
    ROM_API_CRYPTO_TABLE[1])

#define ROM_CRYPTOCcmAuthEncrypt \
    ((uint32_t (*)(bool bEncrypt, uint32_t ui32AuthLength, uint32_t *pui32Nonce, uint32_t *pui32PlainText, uint32_t ui32PlainTextLength, uint32_t *pui32Header, uint32_t ui32HeaderLength, uint32_t ui32KeyLocation, uint32_t ui32FieldLength, bool bIntEnable)) \
    ROM_API_CRYPTO_TABLE[3])

#define ROM_CRYPTOCcmAuthEncryptResultGet \
    ((uint32_t (*)(uint32_t ui32TagLength, uint32_t *pui32CcmTag)) \
    ROM_API_CRYPTO_TABLE[4])

#define ROM_CRYPTOCcmAuthEncryptStatus \
    ((uint32_t (*)(void)) \
    ROM_API_CRYPTO_TABLE[5])

#define ROM_CRYPTOCcmInvAuthDecrypt \
    ((uint32_t (*)(bool bDecrypt, uint32_t ui32AuthLength, uint32_t *pui32Nonce, uint32_t *pui32CipherText, uint32_t ui32CipherTextLength, uint32_t *pui32Header, uint32_t ui32HeaderLength, uint32_t ui32KeyLocation, uint32_t ui32FieldLength, bool bIntEnable)) \
    ROM_API_CRYPTO_TABLE[6])

#define ROM_CRYPTOCcmInvAuthDecryptResultGet \
    ((uint32_t (*)(uint32_t ui32AuthLength, uint32_t *pui32CipherText, uint32_t ui32CipherTextLength, uint32_t *pui32CcmTag)) \
    ROM_API_CRYPTO_TABLE[7])

#define ROM_CRYPTOCcmInvAuthDecryptStatus \
    ((uint32_t (*)(void)) \
    ROM_API_CRYPTO_TABLE[8])

#define ROM_CRYPTOAesCbc \
    ((uint32_t (*)(uint32_t *pui32MsgIn, uint32_t *pui32MsgOut, uint32_t ui32MsgLength, uint32_t *pui32Nonce, uint32_t ui32KeyLocation, bool bEncrypt, bool bIntEnable)) \
    ROM_API_CRYPTO_TABLE[9])

#define ROM_CRYPTOAesCbcStatus \
    ((uint32_t (*)(void)) \
    ROM_API_CRYPTO_TABLE[10])


// OSC FUNCTIONS
#define ROM_OSCClockSourceGet \
    ((uint32_t (*)(uint32_t ui32SrcClk)) \
    ROM_API_OSC_TABLE[0])

#define ROM_OSCClockSourceSet \
    ((void (*)(uint32_t ui32SrcClk, uint32_t ui32Osc)) \
    ROM_API_OSC_TABLE[1])


// SYS_CTRL FUNCTIONS
#define ROM_SysCtrlResetSourceGet \
    ((uint32_t (*)(void)) \
    ROM_API_SYS_CTRL_TABLE[0])


// AON_BATMON FUNCTIONS
#define ROM_AONBatMonTemperatureGetDegC \
    ((int32_t (*)(void)) \
    ROM_API_AON_BATMON_TABLE[0])


// SETUP_ROM FUNCTIONS
#define ROM_SetupAfterColdResetWakeupFromShutDownCfg1 \
    ((void (*)(uint32_t ccfg_ModeConfReg)) \
    ROM_API_SETUP_ROM_TABLE[0])

#define ROM_SetupAfterColdResetWakeupFromShutDownCfg2 \
    ((void (*)(uint32_t ui32Fcfg1Revision, uint32_t ccfg_ModeConfReg)) \
    ROM_API_SETUP_ROM_TABLE[1])

#define ROM_SetupAfterColdResetWakeupFromShutDownCfg3 \
    ((void (*)(uint32_t ccfg_ModeConfReg)) \
    ROM_API_SETUP_ROM_TABLE[2])

#define ROM_SetupGetTrimForAdcShModeEn \
    ((uint32_t (*)(uint32_t ui32Fcfg1Revision)) \
    ROM_API_SETUP_ROM_TABLE[3])

#define ROM_SetupGetTrimForAdcShVbufEn \
    ((uint32_t (*)(uint32_t ui32Fcfg1Revision)) \
    ROM_API_SETUP_ROM_TABLE[4])

#define ROM_SetupGetTrimForAmpcompCtrl \
    ((uint32_t (*)(uint32_t ui32Fcfg1Revision)) \
    ROM_API_SETUP_ROM_TABLE[5])

#define ROM_SetupGetTrimForAmpcompTh1 \
    ((uint32_t (*)(void)) \
    ROM_API_SETUP_ROM_TABLE[6])

#define ROM_SetupGetTrimForAmpcompTh2 \
    ((uint32_t (*)(void)) \
    ROM_API_SETUP_ROM_TABLE[7])

#define ROM_SetupGetTrimForAnabypassValue1 \
    ((uint32_t (*)(uint32_t ccfg_ModeConfReg)) \
    ROM_API_SETUP_ROM_TABLE[8])

#define ROM_SetupGetTrimForDblrLoopFilterResetVoltage \
    ((uint32_t (*)(uint32_t ui32Fcfg1Revision)) \
    ROM_API_SETUP_ROM_TABLE[9])

#define ROM_SetupGetTrimForRadcExtCfg \
    ((uint32_t (*)(uint32_t ui32Fcfg1Revision)) \
    ROM_API_SETUP_ROM_TABLE[10])

#define ROM_SetupGetTrimForRcOscLfIBiasTrim \
    ((uint32_t (*)(uint32_t ui32Fcfg1Revision)) \
    ROM_API_SETUP_ROM_TABLE[11])

#define ROM_SetupGetTrimForRcOscLfRtuneCtuneTrim \
    ((uint32_t (*)(void)) \
    ROM_API_SETUP_ROM_TABLE[12])

#define ROM_SetupGetTrimForXoscHfCtl \
    ((uint32_t (*)(uint32_t ui32Fcfg1Revision)) \
    ROM_API_SETUP_ROM_TABLE[13])

#define ROM_SetupGetTrimForXoscHfFastStart \
    ((uint32_t (*)(void)) \
    ROM_API_SETUP_ROM_TABLE[14])

#define ROM_SetupGetTrimForXoscHfIbiastherm \
    ((uint32_t (*)(void)) \
    ROM_API_SETUP_ROM_TABLE[15])

#define ROM_SetupGetTrimForXoscLfRegulatorAndCmirrwrRatio \
    ((uint32_t (*)(uint32_t ui32Fcfg1Revision)) \
    ROM_API_SETUP_ROM_TABLE[16])

#define ROM_SetupSetAonRtcSubSecInc \
    ((void (*)(uint32_t subSecInc)) \
    ROM_API_SETUP_ROM_TABLE[17])

#define ROM_SetupSetCacheModeAccordingToCcfgSetting \
    ((void (*)(void)) \
    ROM_API_SETUP_ROM_TABLE[18])



//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif // __ROM_H__
