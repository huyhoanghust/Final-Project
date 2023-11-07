/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  SIM   ------> UART1 115200
  RS232 ------> UART2 9600
  GNSS  ------> UART4 9600
  DEBUG ------> UART5 115200

  RFID  ------> SPI1  18Mb/s
  SDCAD ------> SPI3
  FLASH ------> SPI2

  SENSOR -----> TIM1
  MOve/stop --> TIM2
  4G LTE   ---> TIM3
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "i2c.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "sensors.h"
#include "rc522.h"
#include "debug.h"
#include "sim4g_lte.h"
#include "gnss.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
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
uint8_t CardID[5];
char szBuff[20];

enum check
{
  CHANGE,
  NOT_CHANGE
};
volatile int npn_flag_check = NOT_CHANGE;
int npn_count = 0;

enum State
{
	Stop,
	Move
};

int Vehical_State;

float current_time, last_time;
float Vehical_veloc;

//RTC
RTC_TimeTypeDef sTime = {0};
RTC_DateTypeDef sDate = {0};
RTC_AlarmTypeDef sAlarm = {0};
uint8_t sencond = 0;
uint8_t minute = 0;
uint8_t hour = 0;
uint8_t date = 0;
uint8_t day = 0;
uint8_t month = 0;
uint8_t year = 0;

volatile int check =0;

volatile uint8_t buftest[100]={0};
volatile uint8_t buffchar;
volatile int k1=0;

char *GPSResponse;
char latData[15] = {0};
char longData[15] = {0};
char simMessage[200] = {0};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if(huart == &huart1)
  {
    // log_data("RXcallback\n");
    sim7672_callback();
    // buftest[k1++]=buffchar;
    // log_data((char*)&buftest[k1-1]);
    // HAL_UART_Receive_IT(&huart1, (uint8_t *)&buffchar, 1);
    // log_data("RXcallback\n");
  }

  if(huart== &huart4)
  {
    log_data("L70 call\n");
    l70_callback();
  }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  //reset timer 2
  HAL_TIM_Base_Stop(&htim2);
  TIM2 -> CNT = 0;

  current_time = HAL_GetTick();

	npn_flag_check = CHANGE;
  Vehical_State = Move;
  npn_count++;
  if (npn_count==20)
  {
    float time_cycle = current_time-last_time;
    Vehical_veloc=(1.0*3000)/(time_cycle);
    //tinh toan van toc
  }
  //start timer2
  HAL_TIM_Base_Start_IT(&htim2);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if(htim == &htim2)
  {
    npn_flag_check = NOT_CHANGE;
    Vehical_State = Stop;
  }
  if(htim == &htim3)
  {
    log_data("timecallback\n");
    sim7672_timerCallback();
  }
}



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

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_SPI1_Init();
  MX_ADC2_Init();
  MX_TIM1_Init();
  MX_UART5_Init();
  MX_I2C1_Init();
  MX_SPI2_Init();
  MX_SPI3_Init();
  MX_UART4_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */
  log_data("\nGSHT START\n");

  sim7672_pwrkey();
  sim7672_reset();
  sim7672_init();
  sim7672_start_LTE();
  // HAL_UART_Receive_IT(&huart1, (uint8_t *)&buffchar, 1);
    // sim_sendcmd("ATE1\r\n");
    // HAL_Delay(3000);
    // memset(buftest, '\0', sizeof(buftest));
    // sim_sendcmd("AT\r\n");
    // HAL_Delay(3000);
    // memset(buftest, '\0', sizeof(buftest));
    // sim_sendcmd("AT+CFUN?\r\n");
    // HAL_Delay(3000);
    // memset(buftest, '\0', sizeof(buftest));
    // sim_sendcmd("ATE0\r\n");
    // HAL_Delay(3000);
    // memset(buftest, '\0', sizeof(buftest));a
    // sim_sendcmd("AT\r\n");
    // HAL_Delay(3000);
    // memset(buftest, '\0', sizeof(buftest));
    // sim_sendcmd("AT+CFUN?\r\n");
    // HAL_Delay(3000);
  // l70_init();
  //   GPSResponse = l70_receiveGPS();
  // l70_handleGPS(latData, longData, GPSResponse);
  // sprintf(simMessage, "https://www.google.com/maps/search/?api=1&query=%s,%s\n", latData, longData);
  // log_data(simMessage);

  // HAL_UART_Transmit(&huart1, (uint8_t *)"ATE0\r\n", strlen("AT\r\n"), 100);
  // HAL_Delay(200);

  //   //log_data("AT\n");
  //   HAL_UART_Transmit(&huart1, (uint8_t *)"ATE1\r\n", strlen("AT\r\n"), 100);
  //   HAL_Delay(3000);
    // HAL_UART_Transmit(&huart1, (uint8_t *)"AT\r\n", strlen("AT\r\n"), 100);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    //     HAL_UART_Transmit(&huart1, (uint8_t *)"AT+CFUN?\r\n", strlen("AT+CFUN?\r\n"), 100);
    // HAL_Delay(200);
    // // HAL_UART_Transmit(&huart1, (uint8_t *)"AT\r\n", strlen("AT\r\n"), 100);
    // // log_data("AT\n");
    // HAL_UART_Receive(&huart1, (uint8_t*)buftest, sizeof(buftest)-1,2000);
    // log_data(buftest);
    // HAL_Delay(3000);
    //log_data("AT\n");
    // log_data("test...");
    // HAL_Delay(2000);
    //HAL_Delay(2000);
  }
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_ADC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
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
