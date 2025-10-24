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

#define MAX_SAMPLES          200
#define MIN_PEAK_INTERVAL    20

#define MAX30102_I2C_ADDR     0x57
#define MAX30102_MODE_CONFIG  0x09
#define MAX30102_FIFO_DATA    0x07
#define MAX30102_LED_IR_PA1   0x0C
#define MAX30102_FIFO_CONFIG  0x08
#define MAX30102_SPO2_CONFIG  0x0A

#define FIFO_SIZE             32

typedef struct {
    I2C_HandleTypeDef *hi2c;       // I2C 핸들
    uint32_t ir_samples[FIFO_SIZE]; // IR 센서 샘플
} MAX30102_t;

// 초기화 및 레지스터 설정
void MAX30102_Init(MAX30102_t *sensor, I2C_HandleTypeDef *hi2c);
void MAX30102_WriteReg(MAX30102_t *sensor, uint8_t reg, uint8_t data);
void MAX30102_ReadFIFO(MAX30102_t *sensor);

// 샘플 관리
uint32_t MAX30102_GetIRAvg(MAX30102_t *sensor);
void MAX30102_AddSample(uint32_t ir);
uint8_t MAX30102_GetSampleCount(void);
void MAX30102_ResetSamples(void);

// 심박수 계산
uint32_t MAX30102_GetHeartRate(void);
void MAX30102_ResetHeartRate(void);

#endif
