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
#include "fatfs.h"
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
#include "fatfs_sd.h"
#include "mqtt.h"

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

/************************************RTC************************/
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

volatile int check = 0;

volatile uint8_t buftest[100] = {0};
volatile uint8_t buffchar;
volatile int k1 = 0;
/*********************************** GPS************************/
#define GPS 1
char *GPSResponse;
char latData[15] = {0};
char longData[15] = {0};
char simMessage[200] = {0};

char l70Data1;
char gnss_str[100] = {0};

/************************************* SSDCARD*******************/
#define SDCard 0
FATFS fs;                      // file system
FIL fil;                       // file
FRESULT fresult;               // store the result
char SD_buffer_w[1024] = {0};  // write data
char SD_buffer_r[10224] = {0}; // read data
char SD_file_name[50] = {0};
UINT br, bw; // file read.write count

FATFS *pfs;
DWORD fre_clust;
uint32_t total, free_space;

/**************************************SIM 4G***************************/
#define SIM 0
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void set_timer(long time)
{
  TIM3->CNT = 60000 - time * 10;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart == &huart1)
  {
    sim7672_callback();
  }

  if (huart == &huart4)
  {
    l70_callback();
  }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  // reset timer 2
  HAL_TIM_Base_Stop(&htim2);
  TIM2->CNT = 0;

  current_time = HAL_GetTick();

  npn_flag_check = CHANGE;
  Vehical_State = Move;
  npn_count++;
  if (npn_count == 20)
  {
    float time_cycle = current_time - last_time;
    Vehical_veloc = (1.0 * 3000) / (time_cycle);
    // tinh toan van toc
  }
  // start timer2
  HAL_TIM_Base_Start_IT(&htim2);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim == &htim2)
  {
    npn_flag_check = NOT_CHANGE;
    Vehical_State = Stop;
  }
  if (htim == &htim3)
  {

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
  MX_FATFS_Init();
  /* USER CODE BEGIN 2 */
  log_data("\nGSHT START\n");
#if SIM
  // sim7672_pwrOn();
  // HAL_Delay(5000);
  sim7672_init();
  sim7672_check_signalStrength();
  HAL_Delay(500);
  // mqtt_disconnectServer();
  HAL_Delay(1000);
  mqtt_connectServer();
  mqtt_pub("test1z","123456789");
  HAL_Delay(500);
  mqtt_sub("test1z");
#endif

#if GPS
  sim7672_pwrOff();
  l70_init();
  HAL_Delay(1000);
#endif

#if SDCard

  fresult = f_mount(&fs, "/", 1);
  if (fresult != FR_OK)
    send_uart("ERROR!!! in mounting SD CARD...\n\n");
  else
    send_uart("*******************\nSD CARD mounted successfully...\n\n");

  /* ********************************Create second file with read write access and open it */
  fresult = f_open(&fil, "file2.txt", FA_WRITE);
  if (fresult != FR_OK) send_uart ("open error...\n\n");
  else send_uart("open successfully...\n\n");
  // f_lseek(&fil, f_size(&fil));
  f_puts("This data is from the FILE1.txt. And it was written using ...f_puts... \nthis full function", &fil);
  // f_puts("\nThis is updated data and it should be in the end",&fil);
  fresult = f_close(&fil);
  if (fresult != FR_OK) send_uart ("write error...\n\n");
  else send_uart("write successfully...\n\n");
  send_uart ("File2.txt created and data is written\n");
  // clearing buffer to show that result obtained is from the file
  clear_buffer();

  /* ***********************************Open second file to read */
  fresult = f_open(&fil, "file2.txt", FA_READ);
  if (fresult != FR_OK)
    send_uart("open error...\n\n");
  else
    send_uart("open successfully...\n\n");

  /* Read data from the file
   * Please see the function details for the arguments */
  // f_gets(buffer, f_size(&fil), &fil);
  while (f_gets(buffer, sizeof(buffer), &fil) != NULL)
  {
    // Xử lý dữ liệu theo nhu cầu của bạn, ví dụ: gửi qua UART
    send_uart(buffer);
  }
  // f_read (&fil, buffer, f_size(&fil), &br);
  send_uart("data: ");
  send_uart(buffer);
  send_uart("\n\n");

  /* Close file */
  f_close(&fil);

  clear_buffer();
  /* Close file */
  f_close(&fil);
#endif

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    #if GPS
    GPSResponse = l70_receiveGPS();
    l70_handleGPS(GPSResponse);
    HAL_Delay(2000);
    #endif
    // log_data("test...");
    // HAL_Delay(2000);
    // HAL_Delay(2000);
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
