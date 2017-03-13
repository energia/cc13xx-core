/******************************************************************************
*  Filename:       rfc.c
*  Revised:        2016-10-19 12:14:28 +0200 (Wed, 19 Oct 2016)
*  Revision:       47480
*
*  Description:    Driver for the RF Core.
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

#include "rfc.h"
#include <string.h>

//*****************************************************************************
//
// Handle support for DriverLib in ROM:
// This section will undo prototype renaming made in the header file
//
//*****************************************************************************
#if !defined(DOXYGEN)
    #undef  RFCCpeIntGetAndClear
    #define RFCCpeIntGetAndClear            NOROM_RFCCpeIntGetAndClear
    #undef  RFCDoorbellSendTo
    #define RFCDoorbellSendTo               NOROM_RFCDoorbellSendTo
    #undef  RFCSynthPowerDown
    #define RFCSynthPowerDown               NOROM_RFCSynthPowerDown
    #undef  RFCRfTrimRead
    #define RFCRfTrimRead                   NOROM_RFCRfTrimRead
    #undef  RFCRfTrimSet
    #define RFCRfTrimSet                    NOROM_RFCRfTrimSet
    #undef  RFCRTrim
    #define RFCRTrim                        NOROM_RFCRTrim
    #undef  RFCCPEPatchReset
    #define RFCCPEPatchReset                NOROM_RFCCPEPatchReset
    #undef  RFCAdi3VcoLdoVoltageMode
    #define RFCAdi3VcoLdoVoltageMode        NOROM_RFCAdi3VcoLdoVoltageMode
#endif

#define RFC_RESERVED0               0x40044108
#define RFC_RESERVED1               0x40044114
#define RFC_RESERVED2               0x4004410C
#define RFC_RESERVED3               0x40044100

// Position of divider value
#define CONFIG_MISC_ADC_DIVIDER             27
#define CONFIG_MISC_ADC_DIVIDER_BM  0xF8000000U

#define _CPERAM_START 0x21000000
#define _PARSER_PATCH_TAB_OFFSET 0x0350
#define _PATCH_TAB_OFFSET 0x0358
#define _IRQPATCH_OFFSET 0x03E8
#define _PATCH_VEC_OFFSET 0x0448

static const uint16_t rfc_defaultIrqAddr[] =
{
   0x3f17,
   0x3de1,
   0x3e03,
   0x3e17,
   0x0b0d,
   0x3e35,
   0x3e53,
   0x09fb,
   0x655d,
   0x0af3,
   0x3e73,
   0x3ed7,
};

//*****************************************************************************
//
// Get and clear CPE interrupt flags
//
//*****************************************************************************
uint32_t
RFCCpeIntGetAndClear(void)
{
    uint32_t ui32Ifg = HWREG(RFC_DBELL_BASE+RFC_DBELL_O_RFCPEIFG);

    do {
        HWREG(RFC_DBELL_BASE+RFC_DBELL_O_RFCPEIFG) = ~ui32Ifg;
    } while (HWREG(RFC_DBELL_BASE+RFC_DBELL_O_RFCPEIFG) & ui32Ifg);

    return (ui32Ifg);
}


//*****************************************************************************
//
// Send command to doorbell and wait for ack
//
//*****************************************************************************
uint32_t
RFCDoorbellSendTo(uint32_t pOp)
{
    while(HWREG(RFC_DBELL_BASE + RFC_DBELL_O_CMDR) != 0);

    RFCAckIntClear();

    HWREG(RFC_DBELL_BASE+RFC_DBELL_O_CMDR) = pOp;

    while(!HWREG(RFC_DBELL_BASE + RFC_DBELL_O_RFACKIFG));
    RFCAckIntClear();

    return(HWREG(RFC_DBELL_BASE + RFC_DBELL_O_CMDSTA));
}


//*****************************************************************************
//
// Turn off synth, NOTE: Radio will no longer respond to commands!
//
//*****************************************************************************
void
RFCSynthPowerDown()
{
    // Disable CPE clock, enable FSCA clock. NOTE: Radio will no longer respond to commands!
  HWREG(RFC_PWR_NONBUF_BASE + RFC_PWR_O_PWMCLKEN) = (HWREG(RFC_PWR_NONBUF_BASE + RFC_PWR_O_PWMCLKEN) & ~RFC_PWR_PWMCLKEN_CPE_M) | RFC_PWR_PWMCLKEN_FSCA_M;

  HWREG(RFC_RESERVED0) = 3;
  HWREG(RFC_RESERVED1) = 0x1030;
  HWREG(RFC_RESERVED2) = 1;
  HWREG(RFC_RESERVED1) = 0x50;
  HWREG(RFC_RESERVED2) = 1;
  HWREG(RFC_RESERVED1) = 0x650;
  HWREG(RFC_RESERVED2) = 1;
  HWREG(RFC_RESERVED3) = 1;

}


//*****************************************************************************
//
// Read RF Trim from flash using the CM3
//
//*****************************************************************************
void RFCRfTrimRead(rfc_radioOp_t *pOpSetup, rfTrim_t* pRfTrim)
{
    // Read trim from FCFG1
    pRfTrim->configIfAdc = HWREG(FCFG1_BASE + FCFG1_O_CONFIG_IF_ADC);
    pRfTrim->configRfFrontend = HWREG(FCFG1_BASE + FCFG1_O_CONFIG_RF_FRONTEND);
    pRfTrim->configSynth = HWREG(FCFG1_BASE + FCFG1_O_CONFIG_SYNTH);
    // Make sure configMiscAdc is not 0 by setting an unused bit to 1
    pRfTrim->configMiscAdc = (HWREG(FCFG1_BASE + FCFG1_O_CONFIG_MISC_ADC)
                            & ~CONFIG_MISC_ADC_DIVIDER_BM) | (2U << CONFIG_MISC_ADC_DIVIDER);
}


//*****************************************************************************
//
// Check Override RTrim vs FCFG RTrim
//
//*****************************************************************************
void RFCRTrim(rfc_radioOp_t *pOpSetup)
{
 // Function is left blank for compatibility reasons.
}


//*****************************************************************************
//
// Write preloaded RF trim values to CM0
//
//*****************************************************************************
void RFCRfTrimSet(rfTrim_t* pRfTrim)
{
    memcpy((void*)&HWREG(0x21000018), (void*)pRfTrim, sizeof(rfTrim_t));
}


//*****************************************************************************
//
// Reset previously patched CPE RAM to a state where it can be patched again
//
//*****************************************************************************
void RFCCPEPatchReset(void)
{
    uint8_t *pPatchTab = (uint8_t *) (_CPERAM_START + _PARSER_PATCH_TAB_OFFSET);
    uint32_t *pIrqPatch = (uint32_t *) (_CPERAM_START + _IRQPATCH_OFFSET);

    memset(pPatchTab, 0xFF, _IRQPATCH_OFFSET - _PARSER_PATCH_TAB_OFFSET);

	int i;
    for (i = 0; i < sizeof(rfc_defaultIrqAddr)/sizeof(rfc_defaultIrqAddr[0]); i++)
    {
        pIrqPatch[i * 2 + 1] = rfc_defaultIrqAddr[i];
    }
}


//*****************************************************************************
//
// Function to set VCOLDO reference to voltage mode
//
//*****************************************************************************
void RFCAdi3VcoLdoVoltageMode(bool bEnable)
{
 // Function is left blank for compatibility reasons.
}



//*****************************************************************************
//
// Handle support for DriverLib in ROM:
// This section will undo prototype renaming made in the header file
//
//*****************************************************************************
#if !defined(DOXYGEN)
    #undef  RFCCpeIntGetAndClear
    #define RFCCpeIntGetAndClear            NOROM_RFCCpeIntGetAndClear
    #undef  RFCDoorbellSendTo
    #define RFCDoorbellSendTo               NOROM_RFCDoorbellSendTo
    #undef  RFCSynthPowerDown
    #define RFCSynthPowerDown               NOROM_RFCSynthPowerDown
    #undef  RFCRfTrimRead
    #define RFCRfTrimRead                   NOROM_RFCRfTrimRead
    #undef  RFCRfTrimSet
    #define RFCRfTrimSet                    NOROM_RFCRfTrimSet
    #undef  RFCRTrim
    #define RFCRTrim                        NOROM_RFCRTrim
    #undef  RFCCPEPatchReset
    #define RFCCPEPatchReset                NOROM_RFCCPEPatchReset
    #undef  RFCAdi3VcoLdoVoltageMode
    #define RFCAdi3VcoLdoVoltageMode        NOROM_RFCAdi3VcoLdoVoltageMode
#endif

// See rfc.h for implementation
