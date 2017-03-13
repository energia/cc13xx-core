/******************************************************************************
*  Filename:       rf_patch_rfe_ble.h
*  Revised:        $Date$
*  Revision:       $Revision$
*
*  Description: RF core patch for CC2640R2 BLE
*
*  Copyright (c) 2015-2016, Texas Instruments Incorporated
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


#ifndef _RF_PATCH_RFE_BLE_H
#define _RF_PATCH_RFE_BLE_H

#ifdef DEVICE_FAMILY
    #undef DEVICE_FAMILY_PATH
    #define DEVICE_FAMILY_PATH(x) <ti/devices/DEVICE_FAMILY/x>
#else
    #error "You must define DEVICE_FAMILY at the project level as one of cc26x0, cc26x0r2, cc13x0, etc."
#endif

#include <stdint.h>
#include DEVICE_FAMILY_PATH(inc/hw_types.h)

#ifndef RFE_PATCH_TYPE
#define RFE_PATCH_TYPE static const uint32_t
#endif

#ifndef PATCH_FUN_SPEC
#define PATCH_FUN_SPEC static inline
#endif

#ifndef RFC_RFERAM_BASE
#define RFC_RFERAM_BASE 0x2100C000
#endif

#ifndef RFE_PATCH_MODE
#define RFE_PATCH_MODE 0
#endif

RFE_PATCH_TYPE patchBleRfe[452] = { 
   0x00006168,
   0x65f925f8,
   0x004575f9,
   0x00450045,
   0x00260026,
   0x0052a357,
   0x004c0052,
   0x1f401f40,
   0x003f1f40,
   0x00000000,
   0x40004030,
   0x40034001,
   0x400f4007,
   0x40cf404f,
   0x43cf41cf,
   0x4fcf47cf,
   0x2fcf3fcf,
   0x0fcf1fcf,
   0x00000000,
   0x00000000,
   0x000f0000,
   0x00000008,
   0x0000003f,
   0x003f0040,
   0x00040000,
   0x000e0068,
   0x000600dc,
   0x001a0043,
   0x00000005,
   0x00020000,
   0x00000000,
   0x00000000,
   0x00c00004,
   0x00040000,
   0x000000c0,
   0x00000007,
   0x00000000,
   0x00000000,
   0x00000000,
   0x00000000,
   0x40004030,
   0x40034001,
   0x400f4007,
   0x40cf404f,
   0x6fcf7fcf,
   0x4fcf5fcf,
   0x2fcf3fcf,
   0x0fcf1fcf,
   0x00000000,
   0x00000000,
   0x9100c050,
   0xc0707000,
   0x70009100,
   0x00213182,
   0xb1109131,
   0x81017000,
   0xa100b101,
   0x91323182,
   0x9101b110,
   0x81411011,
   0x40772241,
   0x700006f1,
   0x9101c051,
   0x39101830,
   0xd0083183,
   0x6f413118,
   0x91310031,
   0x1483b110,
   0x68831614,
   0x10257000,
   0x9100c050,
   0xc140c3f4,
   0x6f031420,
   0x04411031,
   0x22f08250,
   0x26514098,
   0x3182c022,
   0x91310021,
   0x3963b110,
   0x04411031,
   0x3182c082,
   0x91310021,
   0x3963b110,
   0xc0a21031,
   0x00213182,
   0xb1109131,
   0x31151050,
   0x92051405,
   0x64677000,
   0x1031c052,
   0x31610631,
   0x646a02c1,
   0x1031c112,
   0x06713921,
   0x02e13151,
   0x7000646a,
   0x9101c051,
   0xc0e2cc01,
   0x6467646a,
   0xc0c2c111,
   0xb050646a,
   0x7100b054,
   0x22408040,
   0x220044d6,
   0xb06040c9,
   0x1e118081,
   0x700044c9,
   0xb064a054,
   0x220080f0,
   0x124040c9,
   0xb03290b0,
   0x68dfc170,
   0xc122c111,
   0xc170646a,
   0xc05168e4,
   0xc0e29101,
   0x82603182,
   0x91320002,
   0xc230b110,
   0x825368ee,
   0x39533953,
   0xc36064b1,
   0x128068f4,
   0xb03290b0,
   0x64677000,
   0xc122c101,
   0xc101646a,
   0x646ac0c2,
   0x64b18253,
   0x90b012c0,
   0x7000b032,
   0xc2726467,
   0x646ac081,
   0xc111c122,
   0xc002646a,
   0x646ac111,
   0xc331c062,
   0xc362646a,
   0x646ac111,
   0xc111c302,
   0x8253646a,
   0x64b13953,
   0x646fc3e2,
   0x411c2211,
   0xc881c242,
   0xc252646a,
   0x646ac111,
   0xcee1c272,
   0xc202646a,
   0x646ac881,
   0xc801c202,
   0xc0b0646a,
   0x70006930,
   0xc2426467,
   0x646ac801,
   0xc011c252,
   0xc272646a,
   0x646ac0e1,
   0xc101c002,
   0xc062646a,
   0x646ac301,
   0xc101c122,
   0xc362646a,
   0x646ac101,
   0xc101c302,
   0x8253646a,
   0x700064b1,
   0x1e118081,
   0xb050415c,
   0x7100b054,
   0x22408040,
   0xa054415d,
   0x80f1b064,
   0x45512201,
   0x22007000,
   0xb0604151,
   0x0000614e,
   0x00000000,
   0x00000000,
   0x00000000,
   0x72057306,
   0x720e720b,
   0x7100b050,
   0xb0608081,
   0x8092a050,
   0x92f1eff0,
   0x676a9302,
   0x458c2241,
   0xc1f18080,
   0x16300410,
   0x14011101,
   0x61b56c01,
   0x61b761b6,
   0x61b961b8,
   0x61bb61ba,
   0x61bf61bd,
   0x61c361c1,
   0x632d6326,
   0x0402c0f0,
   0x2a413132,
   0x16321412,
   0x14211101,
   0x61da6c01,
   0x61e061c5,
   0x61cb61c7,
   0x61b561b5,
   0x61f461b5,
   0x61fa61c5,
   0x61cb61c7,
   0x61b561b5,
   0x61f461b5,
   0x61fa61c5,
   0x61cb61c7,
   0x61b561b5,
   0x61e761b5,
   0x61ed61c5,
   0x61cb61c7,
   0x61b561b5,
   0x61cf61b5,
   0x61cf61cf,
   0x61cf61cf,
   0x64c061cf,
   0x64f961cf,
   0x650661cf,
   0x653261cf,
   0x677461cf,
   0x661161cf,
   0x64c061cf,
   0x64f9654e,
   0x650661cf,
   0x65326611,
   0x808261cf,
   0x92f2dfe0,
   0xb0b0676a,
   0xb0b161d6,
   0x72057306,
   0x616cb030,
   0x676acfd0,
   0xc003c284,
   0x647cc3c0,
   0x78517820,
   0x78e37882,
   0x78b67915,
   0xcfc06200,
   0xc284676a,
   0xc3c0c003,
   0x7830647c,
   0x78927861,
   0x791578f3,
   0x620078c6,
   0x676acfb0,
   0xc003c284,
   0x647cc3c0,
   0x78717840,
   0x790378a2,
   0x78d67915,
   0x92219210,
   0x92439232,
   0x92659254,
   0xc01f9156,
   0x394091ff,
   0x100106f0,
   0x14103110,
   0x67709200,
   0xb0e361cf,
   0x80f0a054,
   0x461a2250,
   0x22008040,
   0x621246be,
   0xc800a0e3,
   0x81599160,
   0x8091b050,
   0x468e2241,
   0x676acfa0,
   0x31828212,
   0x39423982,
   0x8212648b,
   0x102f06f2,
   0x142f311f,
   0x22d68216,
   0xc1404633,
   0xc5006234,
   0x6f0d1420,
   0x10de396d,
   0x044ec3f4,
   0x3182c082,
   0x396d002e,
   0x3182c0a2,
   0x821a002d,
   0x06fa398a,
   0x31808220,
   0xc00b3980,
   0x10bc180b,
   0x820318ac,
   0x149b1439,
   0x06f08210,
   0x31101001,
   0x81511410,
   0x140c1410,
   0x46c922c6,
   0x26c1d031,
   0x10026264,
   0x3001c011,
   0x1801c010,
   0x31821802,
   0x26c10021,
   0xb0039191,
   0xb063b013,
   0x8041b053,
   0x46be2201,
   0x91c481b4,
   0x1cb581d5,
   0x18954e77,
   0x80f09165,
   0x42692210,
   0x913d628c,
   0x913eb110,
   0x80e0b110,
   0x46812200,
   0x428122e6,
   0x1895b0e0,
   0x920f9165,
   0x14f98159,
   0x225080f0,
   0x22104211,
   0x634d468c,
   0x676acf90,
   0xa052b063,
   0xc0f28230,
   0x10020420,
   0x3001c011,
   0x1801c010,
   0x31821802,
   0x26c10021,
   0x91919191,
   0xb003b013,
   0xb053b063,
   0xb054b064,
   0x80417100,
   0x46be2201,
   0xb064b063,
   0x225080f0,
   0x81b14211,
   0x81d191c1,
   0x91611891,
   0x62a4b031,
   0xa0e06729,
   0x82058159,
   0xc0801459,
   0xb0637100,
   0x62696aba,
   0x8201a0e3,
   0x31828162,
   0xef803d82,
   0x930292f1,
   0xa003676a,
   0x82377000,
   0x39873947,
   0x39808230,
   0x100206f0,
   0x3001c011,
   0x1801c010,
   0x31821802,
   0x26c10021,
   0xb0029181,
   0x8230b012,
   0x06f03940,
   0xc0111002,
   0xc0103001,
   0x18021801,
   0x00213182,
   0x919126c1,
   0xb013b003,
   0xb053b063,
   0xa0537100,
   0xb052b062,
   0x22018041,
   0x81a446be,
   0x81d591c4,
   0x4efe1cb5,
   0x91651895,
   0x221080f0,
   0x628c42f0,
   0x91c481b4,
   0x184581d4,
   0x4f081c75,
   0x221080f0,
   0x628c42f0,
   0xb110913d,
   0xb110913e,
   0x220080e0,
   0x22e64712,
   0xb0e04312,
   0x920f9165,
   0x14f98159,
   0x225080f0,
   0x22104211,
   0x6330468c,
   0xa0e06729,
   0x82058159,
   0xc0f01459,
   0xb0627100,
   0x62f06b22,
   0x648b80a2,
   0x821261cf,
   0x06f23942,
   0xb050608b,
   0x61cf7100,
   0x676acf70,
   0x80418240,
   0x46be2201,
   0x91c481a4,
   0x189581d5,
   0x80f19165,
   0x468c2211,
   0x80416b33,
   0x46be2201,
   0x91c481a4,
   0x1cc581d5,
   0x18954b1c,
   0x80f09165,
   0x433f2210,
   0xcf60628c,
   0x8240676a,
   0x22018041,
   0x81b446be,
   0x81d591c4,
   0x91651895,
   0x221180f1,
   0x6b50468c,
   0x22018041,
   0x81b446be,
   0x81d591c4,
   0x4ab41cc5,
   0x91651895,
   0x221080f0,
   0x628c435c,
   0x82d092e0,
   0x476b2200,
   0x7000b2c0,
   0x22f080a0,
   0xb0304387,
   0x3162c102,
   0x80a0c001,
   0x43801e00,
   0x438122f0,
   0x3160f880,
   0x63813960,
   0x1a101020,
   0x6e236f13,
   0x16121611,
   0x70006b82
};

PATCH_FUN_SPEC void rf_patch_rfe_ble(void)
{
#ifdef __PATCH_NO_UNROLLING
   uint32_t i;
   for (i = 0; i < 452; i++) {
      HWREG(RFC_RFERAM_BASE + 4 * i) = patchBleRfe[i];
   }
#else
   const uint32_t *pS = patchBleRfe;
   volatile unsigned long *pD = &HWREG(RFC_RFERAM_BASE);
   uint32_t t1, t2, t3, t4, t5, t6, t7, t8;
   uint32_t nIterations = 56;

   do {
      t1 = *pS++;
      t2 = *pS++;
      t3 = *pS++;
      t4 = *pS++;
      t5 = *pS++;
      t6 = *pS++;
      t7 = *pS++;
      t8 = *pS++;
      *pD++ = t1;
      *pD++ = t2;
      *pD++ = t3;
      *pD++ = t4;
      *pD++ = t5;
      *pD++ = t6;
      *pD++ = t7;
      *pD++ = t8;
   } while (--nIterations);

   t1 = *pS++;
   t2 = *pS++;
   t3 = *pS++;
   t4 = *pS++;
   *pD++ = t1;
   *pD++ = t2;
   *pD++ = t3;
   *pD++ = t4;
#endif
}

#endif
