/*
 * Max30102.c
 *
 *  Created on: Oct 15, 2025
 *      Author: Minsig
 */

/*
#include "max30102.h"
#include <stdio.h>
#include <string.h>

void MAX30102_WriteReg(MAX30102_t *sensor, uint8_t reg, uint8_t data) {
    HAL_I2C_Mem_Write(sensor->hi2c, MAX30102_I2C_ADDR << 1, reg, 1, &data, 1, HAL_MAX_DELAY);
}

void MAX30102_Init(MAX30102_t *sensor, I2C_HandleTypeDef *hi2c) {
    sensor->hi2c = hi2c;
    memset(sensor->ir_samples, 0, sizeof(sensor->ir_samples));
    memset(sensor->red_samples, 0, sizeof(sensor->red_samples));

    // 센서 초기화: IR, RED LED 끄고 모드 설정
    MAX30102_WriteReg(sensor, MAX30102_MODE_CONFIG, 0x40);
    HAL_Delay(100);

    MAX30102_WriteReg(sensor, MAX30102_FIFO_CONFIG, 0x4F);

    // SPO2 설정: 100Hz 샘플레이트, 18bit 해상도
    MAX30102_WriteReg(sensor, MAX30102_SPO2_CONFIG, 0x27);

    // LED 최소 전류 설정 (6.4mA 정도)
    MAX30102_SetLEDs(sensor, 6.4f, 6.4f);

    // SPO2 모드 설정
    MAX30102_WriteReg(sensor, MAX30102_MODE_CONFIG, 0x03);
}


void MAX30102_ReadFIFO(MAX30102_t *sensor) {
    uint8_t data[6*FIFO_SIZE]; // 6바이트 * 샘플 수
    HAL_StatusTypeDef ret = HAL_I2C_Mem_Read(sensor->hi2c, MAX30102_I2C_ADDR<<1, MAX30102_FIFO_DATA, 1, data, 6*FIFO_SIZE, HAL_MAX_DELAY);
    if(ret != HAL_OK) printf("I2C Read Failed!\r\n");

    for(int i=0; i<FIFO_SIZE; i++){
        uint32_t ir_sample  = ((uint32_t)data[6*i]<<16) | ((uint32_t)data[6*i+1]<<8) | data[6*i+2];
        uint32_t red_sample = ((uint32_t)data[6*i+3]<<16) | ((uint32_t)data[6*i+4]<<8) | data[6*i+5];

        sensor->ir_samples[i]  = ir_sample & 0x3FFFF;
        sensor->red_samples[i] = red_sample & 0x3FFFF;
    }
}

void MAX30102_SetLEDs(MAX30102_t *sensor, float ir_ma, float red_ma) {
    uint8_t ir_val = (uint8_t)(ir_ma / 0.2f);
    uint8_t red_val = (uint8_t)(red_ma / 0.2f);
    MAX30102_WriteReg(sensor, MAX30102_LED_IR_PA1, ir_val);
    MAX30102_WriteReg(sensor, MAX30102_LED_RED_PA2, red_val);
}

uint32_t MAX30102_GetIRAvg(MAX30102_t *sensor) {
    uint32_t sum=0;
    for(int i=0;i<FIFO_SIZE;i++) sum += sensor->ir_samples[i];
    return sum / FIFO_SIZE;
}

uint32_t MAX30102_GetREDAvg(MAX30102_t *sensor) {
    uint32_t sum=0;
    for(int i=0;i<FIFO_SIZE;i++) sum += sensor->red_samples[i];
    return sum / FIFO_SIZE;
}
*/

#include "max30102.h"
#include <stdio.h>

void MAX30102_WriteReg(MAX30102_t *sensor, uint8_t reg, uint8_t data) {
    HAL_I2C_Mem_Write(sensor->hi2c, MAX30102_I2C_ADDR << 1, reg, 1, &data, 1, HAL_MAX_DELAY);
}

void MAX30102_Init(MAX30102_t *sensor, I2C_HandleTypeDef *hi2c) {
    sensor->hi2c = hi2c;

    // 기본 초기화
    MAX30102_WriteReg(sensor, MAX30102_MODE_CONFIG, 0x40);  // reset
    HAL_Delay(100);
    MAX30102_WriteReg(sensor, MAX30102_FIFO_CONFIG, 0x4F);
    MAX30102_WriteReg(sensor, MAX30102_SPO2_CONFIG, 0x27);
    MAX30102_WriteReg(sensor, MAX30102_LED_IR_PA1, 32);   // IR LED 6.4~6.5 mA
    MAX30102_WriteReg(sensor, MAX30102_LED_RED_PA2, 32);  // RED LED 6.4~6.5 mA
    MAX30102_WriteReg(sensor, MAX30102_MODE_CONFIG, 0x03); // SpO2 모드
}

void MAX30102_ReadFIFO(MAX30102_t *sensor) {
    uint8_t data[6];
    if(HAL_I2C_Mem_Read(sensor->hi2c, MAX30102_I2C_ADDR<<1, MAX30102_FIFO_DATA, 1, data, 6, HAL_MAX_DELAY) == HAL_OK) {
        sensor->ir_samples[0]  = ((uint32_t)data[0]<<16 | (uint32_t)data[1]<<8 | data[2]) & 0x3FFFF;
        sensor->red_samples[0] = ((uint32_t)data[3]<<16 | (uint32_t)data[4]<<8 | data[5]) & 0x3FFFF;
    }
}

uint32_t MAX30102_GetIRAvg(MAX30102_t *sensor) {
    return sensor->ir_samples[0];  // FIFO 하나만 사용
}

uint32_t MAX30102_GetREDAvg(MAX30102_t *sensor) {
    return sensor->red_samples[0]; // FIFO 하나만 사용
}
