/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "esp8266.h"
#include "max30102.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
int __io_putchar(int ch)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)&ch, 1, HAL_MAX_DELAY);
    return ch;
}

void I2C_Scanner(void) {
    for(uint8_t addr=1; addr<128; addr++) {
        if(HAL_I2C_IsDeviceReady(&hi2c1, addr << 1, 1, 1) == HAL_OK) {
            printf("Found device at 0x%02X\r\n", addr);
        } else {
            printf(".");
        }
    }
    printf("\r\nDone scanning\r\n");
}

void I2C_Scanner(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

MAX30102_t sensor;
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
    /* USER CODE BEGIN 1 */
    /* USER CODE END 1 */

    /* MCU Configuration--------------------------------------------------------*/
    HAL_Init();

    /* USER CODE BEGIN Init */
    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */
    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_USART1_UART_Init();
    MX_USART2_UART_Init();
    MX_I2C1_Init();

    /* USER CODE BEGIN 2 */
    printf("Hello\r\n");
    HAL_Delay(1000);

    I2C_Scanner();

    MAX30102_Init(&sensor, &hi2c1);
    HAL_Delay(2000);

    uint32_t last_tick = HAL_GetTick();
    uint32_t start_tick = 0;
    uint8_t first_detected = 1;  // 손가락 처음 감지
    uint8_t measured_done = 0;


    printf("=== ESP8266 TCP 서버 시작 ===\r\n");

    ESP8266_HandleTypeDef esp;

    ESP_Init(&esp, &huart1); // Wi-Fi 연결
    const char *ssid = "KT_GiGA_2G_Wave2_3323";
    const char *pwd = "ke1bfg9832";

    ESP_ConnectWiFi(&esp, ssid, pwd);
    printf("Wi-Fi 연결 시도 완료, ESP 응답을 터미널에서 확인하세요\r\n");
    char ip[16];

    ESP_GetIP(&esp, ip, sizeof(ip));
    printf("ESP IP 확인 완료: %s\r\n", ip);

    ESP_SendCommand(&esp, "AT+CIPSERVER=0", 1000);

    if(ESP_StartTCPServer(&esp, 5000))
    {
        printf("TCP 서버 시작 성공\r\n");
        ESP_SendCommand(&esp, "AT+CIPMUX?", 500);
        ESP_SendCommand(&esp, "AT+CIPSERVER?", 500);
    }
    else
    {
        printf("TCP 서버 시작 실패\r\n");
    }


    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        /* USER CODE END WHILE */
        /* USER CODE BEGIN 3 */

        // FIFO 읽기
        MAX30102_ReadFIFO(&sensor);

        uint32_t ir_avg = MAX30102_GetIRAvg(&sensor);

        // 매 루프마다 샘플 추가
        if(ir_avg > 5000)
        {
            MAX30102_AddSample(ir_avg);

            if(first_detected)
            {
                start_tick = HAL_GetTick();
                first_detected = 0;
            }

            uint32_t elapsed = HAL_GetTick() - start_tick;

            if(!measured_done && elapsed >= 5000 && MAX30102_GetSampleCount() >= 100)
            {
                uint32_t bpm = MAX30102_GetHeartRate();
                MAX30102_PrintIRBuffer();
                printf("심박수 측정 완료: %lu BPM\r\n", bpm);

                char msg[10]; // ESP8266로 전송할 메시지
                snprintf(msg, sizeof(msg), "%lu", bpm);

                measured_done = 1;

                char cmd[30]; // ESP8266 AT 명령어
                snprintf(cmd, sizeof(cmd), "AT+CIPSEND=0,%lu", strlen(msg));
                ESP_SendCommand(&esp, cmd, 500); // ESP8266에 전송 명령 보내기
                HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), 1000); // 심박수 전송
            }
        }
        else
        {
            // 손을 떼면 초기화
            first_detected = 1;
            measured_done = 0;
            MAX30102_ResetSamples();
        }

        HAL_Delay(15);
    }

    /*
    if(HAL_GetTick() - last_tick >= 1000)
    {
        last_tick = HAL_GetTick();
        char msg[50];
        counter++; // 값 1 증가
        snprintf(msg, sizeof(msg), "Counter: %lu\r\n", counter);
        // 다중 연결 모드: 클라이언트 ID=0, 메시지 길이 계산
        char cmd[30];
        snprintf(cmd, sizeof(cmd), "AT+CIPSEND=0,%lu", strlen(msg));
        ESP_SendCommand(&esp, cmd, 500); // 실제 메시지 전송
        HAL_UART_Transmit(&huart1, (uint8_t*)msg, strlen(msg), 1000);
    }
    */

    /* USER CODE END 3 */
}



/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
