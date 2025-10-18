/*
 * Max30102.h
 *
 *  Created on: Oct 15, 2025
 *      Author: Minsig
 */

#ifndef INC_MAX30102_H_
#define INC_MAX30102_H_

#include "stm32f1xx_hal.h"
#include <stdint.h>

#define MAX30102_I2C_ADDR    0x57
#define MAX30102_MODE_CONFIG 0x09
#define MAX30102_FIFO_DATA   0x07
#define MAX30102_LED_IR_PA1  0x0C
#define MAX30102_LED_RED_PA2 0x0D

typedef struct {
    I2C_HandleTypeDef *hi2c;
    uint32_t ir_samples[32];
    uint32_t red_samples[32];
} MAX30102_t;

// 함수 선언
void MAX30102_Init(MAX30102_t *sensor, I2C_HandleTypeDef *hi2c);
void MAX30102_SetLEDs(MAX30102_t *sensor, float ir_ma, float red_ma);
void MAX30102_ReadFIFO(MAX30102_t *sensor);

#endif

