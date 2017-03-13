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

#ifndef __BOARD_H
#define __BOARD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ti/drivers/GPIO.h>
#include <ti/drivers/I2C.h>
#include <ti/drivers/PWM.h>
#include <ti/drivers/SPI.h>
#include <ti/drivers/UART.h>
#include <ti/drivers/Watchdog.h>

#include "CC2650STK.h"

#define Board_initGeneral()     CC2650STK_initGeneral()

/* These #defines allow us to reuse TI-RTOS across other device families */


#define Board_BUZZER            CC2650STK_BUZZER
#define Board_BUZZER_OFF        CC2650STK_BUZZER_OFF
#define Board_BUZZER_ON         CC2650STK_BUZZER_ON

#define Board_CRYPTO0           CC2650STK_CRYPTO0

#define Board_FLASH_CS_OFF      CC2650STK_FLASH_CS_OFF
#define Board_FLASH_CS_ON       CC2650STK_FLASH_CS_ON

#define Board_GPIO_BUTTON0      CC2650STK_GPIO_S1
#define Board_GPIO_BUTTON1      CC2650STK_GPIO_S2
#define Board_GPIO_LED0         CC2650STK_GPIO_LED0
#define Board_GPIO_LED1         CC2650STK_GPIO_LED1
#define Board_GPIO_LED_ON       CC2650STK_GPIO_LED_ON
#define Board_GPIO_LED_OFF      CC2650STK_GPIO_LED_OFF

#define Board_GPTIMER0A         CC2650STK_GPTIMER0A
#define Board_GPTIMER0B         CC2650STK_GPTIMER0B
#define Board_GPTIMER1A         CC2650STK_GPTIMER1A
#define Board_GPTIMER1B         CC2650STK_GPTIMER1B
#define Board_GPTIMER2A         CC2650STK_GPTIMER2A
#define Board_GPTIMER2B         CC2650STK_GPTIMER2B
#define Board_GPTIMER3A         CC2650STK_GPTIMER3A
#define Board_GPTIMER3B         CC2650STK_GPTIMER3B

#define Board_I2C0              CC2650STK_I2C0
#define Board_I2C_TMP           Board_I2C0

#define Board_KEY_LEFT          CC2650STK_KEY_LEFT
#define Board_KEY_RIGHT         CC2650STK_KEY_RIGHT

#define Board_MIC_POWER         CC2650STK_MIC_POWER
#define Board_MIC_POWER_OFF     CC2650STK_MIC_POWER_OFF
#define Board_MIC_POWER_ON      CC2650STK_MIC_POWER_ON

#define Board_PIN_BUTTON0       CC2650STK_KEY_LEFT
#define Board_PIN_BUTTON1       CC2650STK_KEY_RIGHT
#define Board_PIN_LED0          CC2650STK_PIN_LED2
#define Board_PIN_LED1          CC2650STK_PIN_LED1
#define Board_PIN_LED2          CC2650STK_PIN_LED2

#define Board_PDM0              CC2650STK_PDM0

#define Board_PWM0              CC2650STK_PWM0
#define Board_PWM1              CC2650STK_PWM1
#define Board_PWM2              CC2650STK_PWM2
#define Board_PWM3              CC2650STK_PWM3
#define Board_PWM4              CC2650STK_PWM4
#define Board_PWM5              CC2650STK_PWM5
#define Board_PWM6              CC2650STK_PWM6
#define Board_PWM7              CC2650STK_PWM7

#define Board_SPI0              CC2650STK_SPI0
#define Board_SPI1              CC2650STK_SPI1
#define Board_SPI_FLASH_CS      CC2650STK_SPI_FLASH_CS

#define Board_UART0             CC2650STK_UART0

#define Board_WATCHDOG0         CC2650STK_WATCHDOG0

/* Board specific I2C addresses */
#define Board_BMP280_ADDR       (0x77)
#define Board_HDC1000_ADDR      (0x43)
#define Board_MPU9250_ADDR      (0x68)
#define Board_MPU9250_MAG_ADDR  (0x0C)
#define Board_OPT3001_ADDR      (0x45)
#define Board_TMP_ADDR          (0x44)

/*
 * These macros are provided for backwards compatibility.
 * Please use the <Driver>_init functions directly rather
 * than Board_init<Driver>.
 */
#define Board_initGPIO()        GPIO_init()
#define Board_initPWM()         PWM_init()
#define Board_initI2C()         I2C_init()
#define Board_initSPI()         SPI_init()
#define Board_initUART()        UART_init()
#define Board_initWatchdog()    Watchdog_init()

/*
 * These macros are provided for backwards compatibility.
 * Please use the 'Board_PIN_xxx' macros to differentiate
 * them from the 'Board_GPIO_xxx' macros.
 */
#define Board_BUTTON0           Board_PIN_BUTTON0
#define Board_BUTTON1           Board_PIN_BUTTON1
#define Board_LED0              Board_PIN_LED0
#define Board_LED1              Board_PIN_LED1
#define Board_LED2              Board_PIN_LED2
#define Board_LED_ON            Board_GPIO_LED_ON
#define Board_LED_OFF           Board_GPIO_LED_OFF
#define Board_TMP007_ADDR       Board_TMP_ADDR

#ifdef __cplusplus
}
#endif

#endif /* __BOARD_H */
