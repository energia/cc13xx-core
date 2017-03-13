/******************************************************************************
*  Filename:       rf_patch_cpe_bt5.h
*  Revised:        $Date$
*  Revision:       $Revision$
*
*  Description:    RF Core patch file for CC26xx R2
*
*  Copyright (c) 2015, Texas Instruments Incorporated
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

#ifndef _RF_PATCH_CPE_BT5_H
#define _RF_PATCH_CPE_BT5_H

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

#include <stdint.h>
#include <string.h>

#ifndef CPE_PATCH_TYPE
#define CPE_PATCH_TYPE static const uint32_t
#endif

#ifndef SYS_PATCH_TYPE
#define SYS_PATCH_TYPE static const uint32_t
#endif

#ifndef PATCH_FUN_SPEC
#define PATCH_FUN_SPEC static inline
#endif

#ifndef _APPLY_PATCH_TAB
#define _APPLY_PATCH_TAB
#endif


CPE_PATCH_TYPE patchImageBt5[] = {
   0x210004fd,
   0x4c37b570,
   0x46254837,
   0x47803550,
   0xd1092802,
   0x88e13440,
   0x42914a34,
   0x7ce9d104,
   0xd4010689,
   0x80e11d91,
   0xb570bd70,
   0x4a30492d,
   0x30504608,
   0x2b007803,
   0x7983d010,
   0xd10d2b01,
   0x095b7c03,
   0xd00907db,
   0x4d2a7d09,
   0x7f6c74c1,
   0x43202020,
   0x47907768,
   0xbd70776c,
   0x74c121ff,
   0xbd704790,
   0x4824b510,
   0x47804c1e,
   0xd10e2802,
   0x31404621,
   0x4b1d88ca,
   0xd108429a,
   0x09527cca,
   0xd00407d2,
   0x06927d22,
   0x1d9ad401,
   0xbd1080ca,
   0x4814b570,
   0x30404919,
   0x09407cc0,
   0xd00707c0,
   0x48174d14,
   0x43208b2c,
   0x47888328,
   0xbd70832c,
   0xbd704788,
   0x3140490b,
   0x280b7108,
   0x2818d00a,
   0x2825d00a,
   0x282ad00a,
   0x490ed00a,
   0x18400080,
   0x477068c0,
   0x4770480c,
   0x4770480c,
   0x4770480c,
   0x4770480c,
   0x21000144,
   0x0000cb37,
   0x00001404,
   0x0000c8eb,
   0x210000a8,
   0x0000b4f5,
   0x0000ad53,
   0x00002020,
   0x0000df80,
   0x210004d9,
   0x210004ad,
   0x21000473,
   0x2100044d,
};
#define _NWORD_PATCHIMAGE_BT5 70

#define _NWORD_PATCHSYS_BT5 0



#ifndef _BT5_SYSRAM_START
#define _BT5_SYSRAM_START 0x20000000
#endif

#ifndef _BT5_CPERAM_START
#define _BT5_CPERAM_START 0x21000000
#endif

#define _BT5_SYS_PATCH_FIXED_ADDR 0x20000000

#define _BT5_PARSER_PATCH_TAB_OFFSET 0x0350
#define _BT5_PATCH_TAB_OFFSET 0x0358
#define _BT5_IRQPATCH_OFFSET 0x03E8
#define _BT5_PATCH_VEC_OFFSET 0x0448

PATCH_FUN_SPEC void enterBt5CpePatch(void)
{
   uint32_t *pPatchVec = (uint32_t *) (_BT5_CPERAM_START + _BT5_PATCH_VEC_OFFSET);

#if (_NWORD_PATCHIMAGE_BT5 > 0)
   memcpy(pPatchVec, patchImageBt5, sizeof(patchImageBt5));
#endif
}

PATCH_FUN_SPEC void enterBt5SysPatch(void)
{
}

PATCH_FUN_SPEC void configureBt5Patch(void)
{
   uint8_t *pPatchTab = (uint8_t *) (_BT5_CPERAM_START + _BT5_PATCH_TAB_OFFSET);


   pPatchTab[1] = 0;
}

PATCH_FUN_SPEC void applyBt5Patch(void)
{
   enterBt5SysPatch();
   enterBt5CpePatch();
   configureBt5Patch();
}

PATCH_FUN_SPEC void refreshBt5Patch(void)
{
   enterBt5CpePatch();
   configureBt5Patch();
}

PATCH_FUN_SPEC void rf_patch_cpe_bt5(void)
{
   applyBt5Patch();
}


//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif //  _RF_PATCH_CPE_BT5_H

