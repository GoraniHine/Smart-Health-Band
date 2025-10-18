/*
 * esp8266.c
 *
 *  Created on: Oct 15, 2025
 *      Author: Minsig
 */

#include "esp8266.h"
#include <string.h>
#include <stdio.h>

static void ESP_ReadResponse(ESP8266_HandleTypeDef *esp, char *buffer, uint16_t size, uint32_t timeout)
{
    uint16_t idx = 0;
    uint8_t rx;
    uint32_t tickstart = HAL_GetTick();

    while ((HAL_GetTick() - tickstart) < timeout && idx < size-1)
    {
        if(HAL_UART_Receive(esp->huart, &rx, 1, 100) == HAL_OK)
        {
            buffer[idx++] = rx;
            tickstart = HAL_GetTick(); // 수신마다 타이머 리셋
        }
    }
    buffer[idx] = '\0';
}

void ESP_SendCommand(ESP8266_HandleTypeDef *esp, const char *cmd, uint32_t timeout)
{
    HAL_UART_Transmit(esp->huart, (uint8_t*)cmd, strlen(cmd), 1000);
    HAL_UART_Transmit(esp->huart, (uint8_t*)"\r\n", 2, 100);
    char buf[200];
    ESP_ReadResponse(esp, buf, sizeof(buf), timeout);
    printf("ESP 응답: %s\r\n", buf);
}

void ESP_Init(ESP8266_HandleTypeDef *esp, UART_HandleTypeDef *huart)
{
    esp->huart = huart;
    ESP_SendCommand(esp, "AT", 1000);
}

uint8_t ESP_ConnectWiFi(ESP8266_HandleTypeDef *esp, const char *ssid, const char *password)
{
    char cmd[100];
    snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\"", ssid, password);
    ESP_SendCommand(esp, cmd, 10000);
    char buf[200];
    ESP_ReadResponse(esp, buf, sizeof(buf), 10000);
    return (strstr(buf, "OK") != NULL);
}

uint8_t ESP_StartTCPServer(ESP8266_HandleTypeDef *esp, uint16_t port)
{
    // 다중 연결
    ESP_SendCommand(esp, "AT+CIPMUX=1", 1000);

    // 서버 시작
    char cmd[50];
    snprintf(cmd, sizeof(cmd), "AT+CIPSERVER=1,%d", port);
    ESP_SendCommand(esp, cmd, 3000); // 충분한 시간

    // 이제 TCP 서버는 OK 또는 ERROR를 ESP_SendCommand 안에서 확인 가능
    return 1; // 서버 시작 시도 성공으로 반환
}

uint8_t ESP_SendData(ESP8266_HandleTypeDef *esp, const char *data)
{
    char cmd[50];
    snprintf(cmd, sizeof(cmd), "AT+CIPSEND=%d", strlen(data));
    ESP_SendCommand(esp, cmd, 1000);
    HAL_Delay(100);
    ESP_SendCommand(esp, data, 1000);
    return 1;
}

uint8_t ESP_SendDataMulti(ESP8266_HandleTypeDef *esp, uint8_t id, const char *data)
{
    char cmd[50];
    snprintf(cmd, sizeof(cmd), "AT+CIPSEND=%d,%d", id, strlen(data));

    // 1. CIPSEND 명령 보내기
    HAL_UART_Transmit(esp->huart, (uint8_t*)cmd, strlen(cmd), 1000);
    HAL_UART_Transmit(esp->huart, (uint8_t*)"\r\n", 2, 100);

    // 2. '>' 기다리기
    char buf[10];
    ESP_ReadResponse(esp, buf, sizeof(buf), 1000);
    if(strchr(buf, '>') == NULL)
    {
        printf("ESP 전송 준비 안됨: %s\r\n", buf);
        return 0;
    }

    // 3. 데이터 전송
    HAL_UART_Transmit(esp->huart, (uint8_t*)data, strlen(data), 1000);

    // 4. 최종 OK 확인
    ESP_ReadResponse(esp, buf, sizeof(buf), 1000);
    return (strstr(buf, "OK") != NULL);
}

uint8_t ESP_ReceiveData(ESP8266_HandleTypeDef *esp, char *buffer, uint16_t len, uint32_t timeout)
{
    ESP_ReadResponse(esp, buffer, len, timeout);
    return strlen(buffer) > 0;
}

uint8_t ESP_GetIP(ESP8266_HandleTypeDef *esp, char *ip_buffer, uint8_t len)
{
    ESP_SendCommand(esp, "AT+CIFSR", 1000);
    char buf[200];
    ESP_ReadResponse(esp, buf, sizeof(buf), 1000);

    char *p = strstr(buf, "STAIP,\"");
    if(p)
    {
        p += 7;
        char *q = strchr(p, '"');
        if(q)
        {
            uint8_t copy_len = q - p;
            if(copy_len > len-1) copy_len = len-1;
            strncpy(ip_buffer, p, copy_len);
            ip_buffer[copy_len] = '\0';
            return 1;
        }
    }
    return 0;
}
