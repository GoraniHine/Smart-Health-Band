/*
 * Max30102.c
 *
 *  Created on: Oct 15, 2025
 *      Author: Minsig
 */


#include "max30102.h"
#include <stdio.h>

static uint32_t ir_buffer[MAX_SAMPLES];
static uint8_t sample_index = 0;
static uint8_t total_samples = 0; // 누적 샘플 수
static uint32_t last_bpm = 0;

void MAX30102_WriteReg(MAX30102_t *sensor, uint8_t reg, uint8_t data)
{
    HAL_I2C_Mem_Write(sensor->hi2c, MAX30102_I2C_ADDR << 1, reg, 1, &data, 1, HAL_MAX_DELAY);
}

void MAX30102_Init(MAX30102_t *sensor, I2C_HandleTypeDef *hi2c)
{
    sensor->hi2c = hi2c;

    // Reset
    MAX30102_WriteReg(sensor, MAX30102_MODE_CONFIG, 0x40);
    HAL_Delay(100);

    // FIFO, LED 설정
    MAX30102_WriteReg(sensor, MAX30102_FIFO_CONFIG, 0x4F);
    MAX30102_WriteReg(sensor, MAX30102_SPO2_CONFIG, 0x27);
    MAX30102_WriteReg(sensor, MAX30102_LED_IR_PA1, 60);

    // SpO2 모드 (IR만 사용)
    MAX30102_WriteReg(sensor, MAX30102_MODE_CONFIG, 0x03);
    HAL_Delay(50);
}

void MAX30102_ReadFIFO(MAX30102_t *sensor)
{
    uint8_t data[6];
    if (HAL_I2C_Mem_Read(sensor->hi2c, MAX30102_I2C_ADDR << 1, MAX30102_FIFO_DATA, 1, data, 6, 100) == HAL_OK)
    {
        sensor->ir_samples[0] = ((uint32_t)data[0] << 16 | (uint32_t)data[1] << 8 | data[2]) & 0x3FFFF;
    }
}

uint32_t MAX30102_GetIRAvg(MAX30102_t *sensor)
{
    return sensor->ir_samples[0];
}

void MAX30102_AddSample(uint32_t ir)
{
    ir_buffer[sample_index] = ir;
    sample_index = (sample_index + 1) % MAX_SAMPLES;
    if (total_samples < MAX_SAMPLES)
        total_samples++;
}

uint8_t MAX30102_GetSampleCount(void)
{
    return total_samples;
}

void MAX30102_ResetSamples(void)
{
    for (int i = 0; i < MAX_SAMPLES; i++)
        ir_buffer[i] = 0;

    sample_index = 0;
    total_samples = 0;
    last_bpm = 0;
}

uint32_t MAX30102_GetHeartRate(void)
{
    uint8_t peaks = 0;
    int last_peak = -MIN_PEAK_INTERVAL;
    uint32_t ir_min = ir_buffer[0], ir_max = ir_buffer[0];

    // 최소/최대값 찾기
    for (int i = 0; i < MAX_SAMPLES; i++)
    {
        if (ir_buffer[i] < ir_min) ir_min = ir_buffer[i];
        if (ir_buffer[i] > ir_max) ir_max = ir_buffer[i];
    }

    uint32_t diff = ir_max - ir_min;

    // 손 제거 감지: IR 값 변화 거의 없으면 초기화
    if (diff < 2000)
    {
        // 손 제거로 판단될 때만 초기화
        if (ir_max < 10000) // IR 값 전체가 낮으면 손 없음
        {
            MAX30102_ResetSamples();
            return 0;
        }
        // 안정적 IR 값이라면 diff를 최소값으로 고정
        diff = 2000;
    }

    // 피크 임계값 최소 500 이상
    float threshold = diff * 0.20f;
    if (threshold < 200) threshold = 200;

    // 피크 계산
    for (int i = 1; i < MAX_SAMPLES - 1; i++)
    {
        float val = ir_buffer[i];
        if (val > ir_buffer[i - 1] && val > ir_buffer[i + 1] && val > threshold)
        {
            if (i - last_peak >= MIN_PEAK_INTERVAL)
            {
                peaks++;
                last_peak = i;
            }
        }
    }

    // BPM 계산
    float duration_sec = (float)MAX_SAMPLES / 25.0f ; // 샘플링 주기 25Hz 가정
    uint32_t bpm_calc = (uint32_t)(peaks / duration_sec * 60.0f);

    if (bpm_calc < 40) bpm_calc = 40;
    if (bpm_calc > 180) bpm_calc = 180;

    /*
    // 부드러운 필터
    bpm_calc = (bpm_calc + last_bpm * 3) / 4;
    last_bpm = bpm_calc;
    */

    return bpm_calc;
}

void MAX30102_PrintIRBuffer(void)
{
    printf("=== IR Buffer ===\r\n");
    for (int i = 0; i < MAX30102_GetSampleCount(); i++)
    {
        printf("[%d] %lu\r\n", i, ir_buffer[i]);
    }
}
