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
 *  @file       SensorTmp007.c
 *
 *  @brief      Driver for the Texas Instruments TMP007 infra-red thermopile
 *              sensor.
 *  ============================================================================
 */

/* -----------------------------------------------------------------------------
*                                       Includes
* ------------------------------------------------------------------------------
*/
#include "Board.h"
#include "SensorTmp007.h"
#include "SensorUtil.h"
#include "SensorI2C.h"

/* -----------------------------------------------------------------------------
*  Constants and macros
* ------------------------------------------------------------------------------
*/

/* TMP007 register addresses */
#define TMP007_REG_ADDR_VOLTAGE         0x00
#define TMP007_REG_ADDR_LOCAL_TEMP      0x01
#define TMP007_REG_ADDR_CONFIG          0x02
#define TMP007_REG_ADDR_OBJ_TEMP        0x03
#define TMP007_REG_ADDR_STATUS          0x04
#define TMP007_REG_PROD_ID              0x1F

/* TMP007 register values */
#define TMP007_VAL_CONFIG_ON            0x1000  // Sensor on state
#define TMP007_VAL_CONFIG_OFF           0x0000  // Sensor off state
#define TMP007_VAL_CONFIG_RESET         0x8000  // Reset command
#define TMP007_VAL_PROD_ID              0x0078  // Product ID

/* Bit values */
#define CONV_RDY_BIT                    0x4000  // Conversion ready

/* Register length */
#define REGISTER_LENGTH                 2

/* Sensor data size */
#define DATA_SIZE                       4

// Sensor selection/de-selection
#define SENSOR_SELECT()     SensorI2C_select(SENSOR_I2C_0,Board_TMP007_ADDR)
#define SENSOR_DESELECT()   SensorI2C_deselect()

/* -----------------------------------------------------------------------------
*  Local Variables
* ------------------------------------------------------------------------------
*/
static uint8_t buf[DATA_SIZE];
static uint16_t val;

/* -----------------------------------------------------------------------------
*  Public functions
* ------------------------------------------------------------------------------
*/

/*******************************************************************************
 * @fn          SensorTmp007_init
 *
 * @brief       Initialize the temperature sensor driver
 *
 * @return      true if success
 ******************************************************************************/
bool SensorTmp007_init(void)
{
    // Configure sensor
    return SensorTmp007_enable(false);
}

/*******************************************************************************
 * @fn          SensorTmp007_enable
 *
 * @brief       Turn the sensor on/off
 *
 * @return      true if success
 ******************************************************************************/
bool SensorTmp007_enable(bool enable)
{
    bool success;

    if (!SENSOR_SELECT())
    {
        return false;
    }

    if (enable)
    {
        val = TMP007_VAL_CONFIG_ON;
    }
    else
    {
        val = TMP007_VAL_CONFIG_OFF;
    }

    val = SWAP(val);
    success = SensorI2C_writeReg(TMP007_REG_ADDR_CONFIG, (uint8_t*)&val,
                             REGISTER_LENGTH);

    SENSOR_DESELECT();

    return success;
}

/*******************************************************************************
 * @fn          SensorTmp007_read
 *
 * @brief       Read the sensor voltage and sensor temperature registers
 *
 * @param       rawTemp - temperature in 16 bit format
 *
 * @param       rawObjTemp - object temperature in 16 bit format
 *
 * @return      true if valid data
 ******************************************************************************/
bool SensorTmp007_read(uint16_t *rawTemp, uint16_t *rawObjTemp)
{
    bool success;

    if (!SENSOR_SELECT())
    {
        return false;
    }

    success = SensorI2C_readReg(TMP007_REG_ADDR_STATUS, (uint8_t *)&val,
                            REGISTER_LENGTH);

    if (success)
    {
        val = SWAP(val);
        success = !!(val & CONV_RDY_BIT);
    }

    if (success)
    {
        // Read the sensor registers
        success = SensorI2C_readReg(TMP007_REG_ADDR_LOCAL_TEMP, &buf[0],
                                REGISTER_LENGTH);
        if (success)
        {
            success = SensorI2C_readReg(TMP007_REG_ADDR_OBJ_TEMP, &buf[2],
                                    REGISTER_LENGTH);
            // Swap bytes
            *rawTemp = buf[0]<<8 | buf[1];
            *rawObjTemp = buf[2]<<8 | buf[3];
        }
    }

    SENSOR_DESELECT();

    return success;
}

/*******************************************************************************
 * @fn          SensorTmp007_test
 *
 * @brief       Run a sensor self-test
 *
 * @return      true if passed
 ******************************************************************************/
bool SensorTmp007_test(void)
{
    // Select this sensor on the I2C bus
    if (!SENSOR_SELECT())
    {
        return false;
    }

    // Check product ID
    ST_ASSERT(SensorI2C_readReg(TMP007_REG_PROD_ID, (uint8_t *)&val, REGISTER_LENGTH));
    val = SWAP(val);
    ST_ASSERT(val == TMP007_VAL_PROD_ID);

    // Turn sensor on
    val = SWAP(TMP007_VAL_CONFIG_ON);
    ST_ASSERT(SensorI2C_writeReg(TMP007_REG_ADDR_CONFIG, (uint8_t *)&val,
                             REGISTER_LENGTH));

    // Check config register (on)
    ST_ASSERT(SensorI2C_readReg(TMP007_REG_ADDR_CONFIG, (uint8_t *)&val,
                            REGISTER_LENGTH));
    val = SWAP(val);
    ST_ASSERT(val == TMP007_VAL_CONFIG_ON);

    // Turn sensor off
    val = SWAP(TMP007_VAL_CONFIG_OFF);
    ST_ASSERT(SensorI2C_writeReg(TMP007_REG_ADDR_CONFIG, (uint8_t *)&val,
                             REGISTER_LENGTH));

    // Check config register (off)
    ST_ASSERT(SensorI2C_readReg(TMP007_REG_ADDR_CONFIG, (uint8_t *)&val,
                            REGISTER_LENGTH));
    val = SWAP(val);
    ST_ASSERT(val == TMP007_VAL_CONFIG_OFF);

    SENSOR_DESELECT();

    return true;
}

/*******************************************************************************
 * @fn          SensorTmp007_convert
 *
 * @brief       Convert raw data to object and ambience temperature
 *
 * @param       rawTemp - raw temperature from sensor
 *
 * @param       rawObjTemp - raw temperature from sensor
 *
 * @param       tObj - converted object temperature
 *
 * @param       tTgt - converted ambience temperature
 *
 * @return      none
 ******************************************************************************/
void SensorTmp007_convert(uint16_t rawTemp, uint16_t rawObjTemp, float *tObj,
                         float *tTgt)
{
    const float SCALE_LSB = 0.03125;
    float t;
    int it;

    it = (int)((rawObjTemp) >> 2);
    t = ((float)(it)) * SCALE_LSB;
    *tObj = t;

    it = (int)((rawTemp) >> 2);
    t = (float)it;
    *tTgt = t * SCALE_LSB;
}
