/*
 * esp8266.h
 *
 *  Created on: Oct 15, 2025
 *      Author: Minsig
 */

#ifndef ESP8266_H
#define ESP8266_H

#include "stm32f1xx_hal.h"
#include <stdint.h>

// ESP8266 핸들 구조체
typedef struct {
    UART_HandleTypeDef *huart;
} ESP8266_HandleTypeDef;

// 초기화
void ESP_Init(ESP8266_HandleTypeDef *esp, UART_HandleTypeDef *huart);

// AT 명령 전송 및 응답 받기
void ESP_SendCommand(ESP8266_HandleTypeDef *esp, const char *cmd, uint32_t timeout);
uint8_t ESP_ReceiveData(ESP8266_HandleTypeDef *esp, char *buffer, uint16_t len, uint32_t timeout);

// Wi-Fi 연결
uint8_t ESP_ConnectWiFi(ESP8266_HandleTypeDef *esp, const char *ssid, const char *password);

// TCP 서버 시작
uint8_t ESP_StartTCPServer(ESP8266_HandleTypeDef *esp, uint16_t port);

// 서버 → 클라이언트 데이터 송신
uint8_t ESP_SendData(ESP8266_HandleTypeDef *esp, const char *data);
uint8_t ESP_SendDataMulti(ESP8266_HandleTypeDef *esp, uint8_t id, const char *data);
// ESP IP 확인
uint8_t ESP_GetIP(ESP8266_HandleTypeDef *esp, char *ip_buffer, uint8_t len);

#endif
