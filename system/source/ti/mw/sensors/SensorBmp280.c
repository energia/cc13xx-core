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
 *  @file       SensorBmp280.c
 *
 *  @brief      Driver for the Bosch BMP280 Pressure Sensor.
 *  ============================================================================
 */

/* -----------------------------------------------------------------------------
*  Includes
* ------------------------------------------------------------------------------
*/
#include "Board.h"
#include "SensorBmp280.h"
#include "SensorUtil.h"
#include "SensorI2C.h"

/* -----------------------------------------------------------------------------
*  Constants and macros
* ------------------------------------------------------------------------------
*/

// Registers
#define ADDR_CALIB                          0x88
#define ADDR_PROD_ID                        0xD0
#define ADDR_RESET                          0xE0
#define ADDR_STATUS                         0xF3
#define ADDR_CTRL_MEAS                      0xF4
#define ADDR_CONFIG                         0xF5
#define ADDR_PRESS_MSB                      0xF7
#define ADDR_PRESS_LSB                      0xF8
#define ADDR_PRESS_XLSB                     0xF9
#define ADDR_TEMP_MSB                       0xFA
#define ADDR_TEMP_LSB                       0xFB
#define ADDR_TEMP_XLSB                      0xFC

// Reset values
#define VAL_PROD_ID                         0x58
#define VAL_RESET                           0x00
#define VAL_STATUS                          0x00
#define VAL_CTRL_MEAS                       0x00
#define VAL_CONFIG                          0x00
#define VAL_PRESS_MSB                       0x80
#define VAL_PRESS_LSB                       0x00
#define VAL_TEMP_MSB                        0x80
#define VAL_TEMP_LSB                        0x00

// Test values
#define VAL_RESET_EXECUTE                   0xB6
#define VAL_CTRL_MEAS_TEST                  0x55

// Misc.
#define CALIB_DATA_SIZE                     24

#define RES_OFF                             0
#define RES_ULTRA_LOW_POWER                 1
#define RES_LOW_POWER                       2
#define RES_STANDARD                        3
#define RES_HIGH                            5
#define RES_ULTRA_HIGH                      6

// Bit fields in CTRL_MEAS register
#define PM_OFF                              0
#define PM_FORCED                           1
#define PM_NORMAL                           3

#define OSRST(v)                            ((v) << 5)
#define OSRSP(v)                            ((v) << 2)

// Sensor selection/deselection
#define SENSOR_SELECT()     SensorI2C_select(SENSOR_I2C_0,Board_BMP280_ADDR)
#define SENSOR_DESELECT()   SensorI2C_deselect()

/* -----------------------------------------------------------------------------
*  Type Definitions
* ------------------------------------------------------------------------------
*/
typedef struct
{
    uint16_t dig_T1;
    int16_t dig_T2;
    int16_t dig_T3;
    uint16_t dig_P1;
    int16_t dig_P2;
    int16_t dig_P3;
    int16_t dig_P4;
    int16_t dig_P5;
    int16_t dig_P6;
    int16_t dig_P7;
    int16_t dig_P8;
    int16_t dig_P9;
    int32_t t_fine;
} Bmp280Calibration_t;

/* -----------------------------------------------------------------------------
*  Local Variables
* ------------------------------------------------------------------------------
*/
static uint8_t calData[CALIB_DATA_SIZE];

/* -----------------------------------------------------------------------------
*  Public functions
* ------------------------------------------------------------------------------
*/

/*******************************************************************************
 * @fn          SensorBmp280_init
 *
 * @brief       Initialize the sensor
 *
 * @return      true if success
 */
bool SensorBmp280_init(void)
{
    bool ret;
    uint8_t val;

    if (!SENSOR_SELECT())
    {
        return false;
    }

    // Read and store calibration data
    ret = SensorI2C_readReg(ADDR_CALIB, calData, CALIB_DATA_SIZE);

    if (ret)
    {
        // Reset the sensor
        val = VAL_RESET_EXECUTE;
        SensorI2C_writeReg(ADDR_RESET, &val, sizeof(val));
    }

    SENSOR_DESELECT();

    return ret;
}


/*******************************************************************************
 * @fn          SensorBmp280_enable
 *
 * @brief       Enable/disable measurements
 *
 * @param       enable - flag to turn the sensor on/off
 *
 * @return      none
 */
void SensorBmp280_enable(bool enable)
{
    uint8_t val;

    if (enable)
    {
        // Enable forced mode; pressure oversampling 4; temp. oversampling 1
        val = PM_NORMAL | OSRSP(4) | OSRST(1);
    }
    else
    {
        val = PM_OFF;
    }

    if (!SENSOR_SELECT())
    {
        return;
    }

    SensorI2C_writeReg(ADDR_CTRL_MEAS, &val, sizeof(val));
    SENSOR_DESELECT();
}


/*******************************************************************************
 * @fn          SensorBmp280_read
 *
 * @brief       Read temperature and pressure data
 *
 * @param       data - buffer for temperature and pressure (6 bytes)
 *
 * @return      TRUE if valid data
 */
bool SensorBmp280_read(uint8_t *data)
{
    bool success;

    if (!SENSOR_SELECT())
    {
        return false;
    }

    success = SensorI2C_readReg(ADDR_PRESS_MSB, data, SENSOR_BMP280_DATASIZE);
    SENSOR_DESELECT();

    if (success)
    {
        // Validate data
        success = !(data[0]==0x80 && data[1]==0x00 && data[2]==0x00);
    }

    return success;
}


/*******************************************************************************
 * @fn          SensorBmp280_convert
 *
 * @brief       Convert raw data to object and ambience temperature
 *
 * @param       data - raw data from sensor
 *
 * @param       temp - converted temperature
 *
 * @param       press - converted pressure
 *
 * @return      none
 ******************************************************************************/
void SensorBmp280_convert(uint8_t *data, int32_t *temp, uint32_t *press)
{
    int32_t utemp, upress;
    Bmp280Calibration_t *p = (Bmp280Calibration_t *)calData;
    int32_t v_x1_u32r;
    int32_t v_x2_u32r;
    int32_t t_fine;
    uint32_t pressure;

    // Pressure
    upress = (int32_t)((((uint32_t)(data[0])) << 12) |
                       (((uint32_t)(data[1])) << 4) | ((uint32_t)data[2] >> 4));

    // Temperature
    utemp = (int32_t)((( (uint32_t) (data[3])) << 12) |
                      (((uint32_t)(data[4])) << 4) | ((uint32_t)data[5] >> 4));

    // Compensate temperature
    v_x1_u32r  = ((((utemp >> 3) - ((int32_t)p->dig_T1 << 1))) *
                  ((int32_t)p->dig_T2)) >> 11;
    v_x2_u32r  = (((((utemp >> 4) - ((int32_t)p->dig_T1)) *
                    ((utemp >> 4) - ((int32_t)p->dig_T1))) >> 12) *
                  ((int32_t)p->dig_T3)) >> 14;
    t_fine = v_x1_u32r + v_x2_u32r;
    *temp = (int32_t)((t_fine * 5 + 128) >> 8);

    // Compensate pressure
    v_x1_u32r = (((int32_t)t_fine) >> 1) - (int32_t)64000;
    v_x2_u32r = (((v_x1_u32r >> 2) * (v_x1_u32r >> 2)) >> 11) *
        ((int32_t)p->dig_P6);
    v_x2_u32r = v_x2_u32r + ((v_x1_u32r * ((int32_t)p->dig_P5)) << 1);
    v_x2_u32r = (v_x2_u32r >> 2) + (((int32_t)p->dig_P4) << 16);
    v_x1_u32r = (((p->dig_P3 * (((v_x1_u32r >> 2) *
                                 (v_x1_u32r >> 2)) >> 13)) >> 3) +
                 ((((int32_t)p->dig_P2) * v_x1_u32r) >> 1)) >> 18;
    v_x1_u32r = ((((32768+v_x1_u32r)) * ((int32_t)p->dig_P1))    >> 15);

    if (v_x1_u32r == 0)
        return; /* Avoid exception caused by division by zero */

    pressure = (((uint32_t)(((int32_t)1048576) - upress) -
                 (v_x2_u32r >> 12))) * 3125;
    if (pressure < 0x80000000)
        pressure = (pressure << 1) / ((uint32_t)v_x1_u32r);
    else
        pressure = (pressure / (uint32_t)v_x1_u32r) * 2;
    v_x1_u32r = (((int32_t)p->dig_P9) *
                 ((int32_t)(((pressure >> 3) * (pressure >> 3)) >> 13))) >> 12;
    v_x2_u32r = (((int32_t)(pressure >> 2)) * ((int32_t)p->dig_P8)) >> 13;
    pressure = (uint32_t)((int32_t)pressure +
                          ((v_x1_u32r + v_x2_u32r + p->dig_P7) >> 4));

    *press = pressure;
}

/*******************************************************************************
 * @fn          SensorBmp280_test
 *
 * @brief       Run a sensor self-test
 *
 * @return      true if passed
 */
bool SensorBmp280_test(void)
{
    uint8_t val;

    // Select this sensor on the I2C bus
    if (!SENSOR_SELECT())
    {
        return false;
    }

    // Check reset values
    ST_ASSERT(SensorI2C_readReg(ADDR_PROD_ID, &val, sizeof(val)));
    ST_ASSERT(val == VAL_PROD_ID);

    ST_ASSERT(SensorI2C_readReg(ADDR_CONFIG, &val, sizeof(val)));
    ST_ASSERT(val == VAL_CONFIG);

    // Check that registers can be written
    val = VAL_CTRL_MEAS_TEST;
    ST_ASSERT(SensorI2C_writeReg(ADDR_CTRL_MEAS, &val, sizeof(val)));
    ST_ASSERT(SensorI2C_readReg(ADDR_CTRL_MEAS, &val, sizeof(val)));
    ST_ASSERT(val == VAL_CTRL_MEAS_TEST);

    // Reset the sensor
    val = VAL_RESET_EXECUTE;
    ST_ASSERT(SensorI2C_writeReg(ADDR_RESET, &val, sizeof(val)));

    // Check that CTRL_MEAS register has reset value
    ST_ASSERT(SensorI2C_readReg(ADDR_CTRL_MEAS, &val, sizeof(val)));
    ST_ASSERT(val == VAL_CTRL_MEAS);

    SENSOR_DESELECT();

    return true;
}
