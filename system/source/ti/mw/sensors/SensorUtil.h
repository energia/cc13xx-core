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
 *  @file       SensorUtil.h
 *
 *  @brief      Common utility code for SensorTag
 *
 *  ============================================================================
 */
#ifndef SENSOR_UTIL_H
#define SENSOR_UTIL_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Task.h>

/*********************************************************************
 * CONSTANTS and MACROS
 */

/* Boolean assertion; return false Failed) and release I2C if condition is not met */
#define ST_ASSERT(cond)   ST( if (!(cond)) {SensorI2C_deselect(); return false;})

/* Void assertion; return and release I2C if condition is not met */
#define ST_ASSERT_V(cond) ST( if (!(cond)) {SensorI2C_deselect(); return;} )

/* Loop enclosure for macros */
#define ST(x)             do { x } while (__LINE__ == -1)

/* Conversion macros */
#define HI_UINT16(a)     (((a) >> 8) & 0xFF)
#define LO_UINT16(a)     ((a) & 0xFF)
#define SWAP(v)          ((LO_UINT16(v) << 8) | HI_UINT16(v))

/* Delay */
#define DELAY_MS(i)      (Task_sleep(((i) * 1000) / Clock_tickPeriod))
#define DELAY_US(i)      (Task_sleep(((i) * 1) / Clock_tickPeriod))
#define MS_2_TICKS(ms)   (((ms) * 1000) / Clock_tickPeriod)

/*********************************************************************
* FUNCTIONS
*/
void     SensorUtil_convertToLe(uint8_t *data, uint8_t len);
uint16_t SensorUtil_floatToSfloat(float data);
float    SensorUtil_sfloatToFloat(uint16_t rawData);
uint16_t SensorUtil_intToSfloat(int data);

/*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* SENSOR_UTIL_H */
