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
 *  @file       SensorTagTest.c
 *
 *  @brief      Power on Self Test for sensors and external flash.
 *  ============================================================================
 */

/* -----------------------------------------------------------------------------
*  Includes
* ------------------------------------------------------------------------------
*/
#include "SensorTagTest.h"
#include "ExtFlash.h"
#include "SensorTmp007.h"
#include "SensorHdc1000.h"
#include "SensorBmp280.h"
#include "SensorOpt3001.h"
#include "SensorMpu9250.h"

/* -----------------------------------------------------------------------------
*  Local Variables
* ------------------------------------------------------------------------------
*/
static uint8_t selfTestResult;

/* -----------------------------------------------------------------------------
*  Public Functions
* ------------------------------------------------------------------------------
*/

/*******************************************************************************
* @fn      SensorTag_testExecute
*
* @brief   Run a self-test on all the sensors
*
* @param   testMap - bit map of sensor to include in the test
*
* @return  bit-mask of passed flags, one bit set for each sensor
*/
uint8_t SensorTag_testExecute(uint8_t testMap)
{
    selfTestResult = 0;

    // 1. Temp sensor test
    if (testMap & SENSOR_TMP_TEST_BM)
    {
        if (SensorTmp007_test())
        {
            selfTestResult |= SENSOR_TMP_TEST_BM;
        }
    }

    // 2. Humidity  sensor test
    if (testMap & SENSOR_HUM_TEST_BM)
    {
        if (SensorHdc1000_test())
        {
            selfTestResult |= SENSOR_HUM_TEST_BM;
        }
    }

    // 3. Barometer test
    if (testMap & SENSOR_BAR_TEST_BM)
    {
        if (SensorBmp280_test())
        {
            selfTestResult |= SENSOR_BAR_TEST_BM;
        }
    }

    // 3. Optic sensor test
    if (testMap & SENSOR_OPT_TEST_BM)
    {
        if (SensorOpt3001_test())
        {
            selfTestResult |= SENSOR_OPT_TEST_BM;
        }
    }

    // 4. MPU test
    if (testMap & SENSOR_MOV_TEST_BM)
    {
        if (SensorMpu9250_test())
        {
            selfTestResult |= SENSOR_MOV_TEST_BM;
        }
    }

    // 5. Magnetometer test
    if (testMap & SENSOR_MAG_TEST_BM)
    {
        if (SensorMpu9250_magTest())
        {
            selfTestResult |= SENSOR_MAG_TEST_BM;
        }
    }

    // 6. Flash test
    if (testMap & SENSOR_FLASH_TEST_BM)
    {
        if (ExtFlash_test())
        {
            selfTestResult |= SENSOR_FLASH_TEST_BM;
        }
    }

    return selfTestResult;
}
