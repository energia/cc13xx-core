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
 *  @file       SensorTagTest.h
 *
 *  @brief      Test code for SensorTag
 *
 *  ============================================================================
 */
#ifndef SENSORTAG_TEST_H
#define SENSORTAG_TEST_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "stdbool.h"
#include "stdint.h"

/*********************************************************************
 * CONSTANTS and MACROS
 */

/* Bit values for power on self-test */
#define SENSOR_TMP_TEST_BM            0x01
#define SENSOR_HUM_TEST_BM            0x02
#define SENSOR_OPT_TEST_BM            0x04
#define SENSOR_BAR_TEST_BM            0x08
#define SENSOR_MOV_TEST_BM            0x10
#define SENSOR_MAG_TEST_BM            0x20
#define SENSOR_FLASH_TEST_BM          0x40

/* Test maps for sensor devices */
#define ST_TEST_MAP_SENSORS ( \
    SENSOR_TMP_TEST_BM | SENSOR_HUM_TEST_BM | SENSOR_OPT_TEST_BM | \
    SENSOR_BAR_TEST_BM | SENSOR_MOV_TEST_BM | SENSOR_MAG_TEST_BM )

/* Test maps for all devices */
#define ST_TEST_MAP  (ST_TEST_MAP_SENSORS | SENSOR_FLASH_TEST_BM)

/*********************************************************************
 * FUNCTIONS
 */
uint8_t  SensorTag_testExecute(uint8_t testMap);

/*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* SENSORTAG_TEST_H */
