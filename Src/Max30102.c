/*
 * Max30102.c
 *
 *  Created on: Oct 15, 2025
 *      Author: Minsig
 */

#include "max30102.h"
#include <string.h>

void MAX30102_WriteReg(MAX30102_t *sensor, uint8_t reg, uint8_t data) {
    HAL_I2C_Mem_Write(sensor->hi2c, MAX30102_I2C_ADDR << 1, reg, 1, &data, 1, HAL_MAX_DELAY);
}

void MAX30102_Init(MAX30102_t *sensor, I2C_HandleTypeDef *hi2c) {
    sensor->hi2c = hi2c;
    memset(sensor->ir_samples, 0, sizeof(sensor->ir_samples));
    memset(sensor->red_samples, 0, sizeof(sensor->red_samples));

    // 센서 초기화: IR, RED LED 끄고 모드 설정
    MAX30102_SetLEDs(sensor, 0, 0);
    MAX30102_WriteReg(sensor, MAX30102_MODE_CONFIG, 0x03); // SPO2 모드
}

void MAX30102_SetLEDs(MAX30102_t *sensor, float ir_ma, float red_ma) {
    uint8_t ir_val = (uint8_t)(ir_ma / 0.2f);
    uint8_t red_val = (uint8_t)(red_ma / 0.2f);
    MAX30102_WriteReg(sensor, MAX30102_LED_IR_PA1, ir_val);
    MAX30102_WriteReg(sensor, MAX30102_LED_RED_PA2, red_val);
}

void MAX30102_ReadFIFO(MAX30102_t *sensor) {
    uint8_t data[6];
    HAL_StatusTypeDef ret;
    ret = HAL_I2C_Mem_Read(sensor->hi2c, MAX30102_I2C_ADDR << 1, MAX30102_FIFO_DATA, 1, data, 6, HAL_MAX_DELAY);
    if(ret != HAL_OK) printf("I2C Read Failed!\r\n");

    uint32_t ir_sample  = ((uint32_t)data[0] << 16) | ((uint32_t)data[1] << 8) | data[2];
    uint32_t red_sample = ((uint32_t)data[3] << 16) | ((uint32_t)data[4] << 8) | data[5];

    sensor->ir_samples[0] = ir_sample & 0x3FFFF;
    sensor->red_samples[0] = red_sample & 0x3FFFF;
}
