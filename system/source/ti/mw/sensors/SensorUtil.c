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
 *  @file       SensorUtil.c
 *
 *  @brief      Common utility code for sensors.
 *
 *  ============================================================================
 */

/* -----------------------------------------------------------------------------
*  Includes
* ------------------------------------------------------------------------------
*/
#include "SensorUtil.h"
#include "math.h"
#include "stdbool.h"

/* -----------------------------------------------------------------------------
*  Macros and constants
* ------------------------------------------------------------------------------
*/
#define PRECISION 100.0
#define IPRECISION 100

/* -----------------------------------------------------------------------------
*  Public functions
* ------------------------------------------------------------------------------
*/

/*******************************************************************************
* @fn      SensorUtil_convertToLe
*
* @brief   Convert 16-bit words form big-endian to little-endian
*
* @param   none
*
* @return  none
*/
void SensorUtil_convertToLe(uint8_t *data, uint8_t len)
{
    uint8_t i;

    for (i=0; i<len; i+=2)
    {
        uint8_t tmp;
        tmp = data[i];
        data[i] = data[i+1];
        data[i+1] = tmp;
    }
}

/*******************************************************************************
* @fn      SensorUtil_floatToSfloat
*
* @brief   Convert a float to a short float
*
* @param   data - floating point number to convert
*
* @return  converted value
*/
uint16_t SensorUtil_floatToSfloat(float data)
{
    double sgn = data > 0 ? +1 : -1;
    double mantissa = fabs(data) * PRECISION;
    int exponent = 0;
    bool scaled = false;

    // Scale if mantissa is too large
    while (!scaled)
    {
        if (mantissa <= (float)0xFFF)
        {
            scaled = true;
        }
        else
        {
            exponent++;
            mantissa /= 2.0;
        }
    }

    uint16_t int_mantissa = (int) round(sgn * mantissa);
    uint16_t sfloat = ((exponent & 0xF) << 12) | (int_mantissa & 0xFFF);

    return sfloat;
}

/*******************************************************************************
* @fn      SensorUtil_floatToSfloat
*
* @brief   Convert a short float to a float
*
* @param   data - floating point number to convert
*
* @return  converted value
*/
float SensorUtil_sfloatToFloat(uint16_t rawData)
{
    uint16_t e, m;

    m = rawData & 0x0FFF;
    e = (rawData & 0xF000) >> 12;

    return m * exp2(e) * (1.0/PRECISION);
}

/*******************************************************************************
* @fn      SensorTagUtil_intToSfloat
*
* @brief   Convert an integer to a short float
*
* @param   data - integer to convert
*
* @return  converted value
*/
uint16_t SensorUtil_intToSfloat(int data)
{
    int sgn = data > 0 ? +1 : -1;
    int mantissa = data * IPRECISION;
    int exponent = 0;
    bool scaled = false;

    // Scale if mantissa is too large
    while (!scaled)
    {
        if (mantissa <= 0xFFF)
        {
            scaled = true;
        }
        else
        {
            exponent++;
            mantissa /= 2;
        }
    }

    uint16_t int_mantissa = sgn * mantissa;
    uint16_t sfloat = ((exponent & 0xF) << 12) | (int_mantissa & 0xFFF);

    return sfloat;
}

/*******************************************************************************
*******************************************************************************/
