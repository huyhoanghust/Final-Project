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
  RFID      --> TIM2
  Task     ---> TIM3
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
#include "flash.h"
#include "ssd1306.h"
#include "bitmap.h"

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
/************************************RFID*********************/
uint8_t CardID[5];
char szBuff[20];
volatile bool waiting4LogOut = false;
volatile bool Logout = false;
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
RTC_TimeTypeDef current_sTime = {0};
RTC_DateTypeDef current_sDate = {0};
RTC_TimeTypeDef last_sTime = {0};
RTC_DateTypeDef last_sDate = {0};
RTC_AlarmTypeDef sAlarm = {0};
uint8_t sencond = 0;
uint8_t minute = 0;
uint8_t hour = 0;
uint8_t date = 0;
uint8_t day = 0;
uint8_t month = 0;
uint8_t year = 0;
char time_buff[20];
char date_buff[20];
volatile int check = 0;

uint16_t totalMinute_OperateTime;
int time_hour;
int time_minute;
char totalMinute_OperateTime_buff[30] = {0};

int numCheck_time = 0;
/*********************************** GPS************************/
#define GPS 0
char *GPSResponse;
char latData[15] = {0};
char longData[15] = {0};
char simMessage[200] = {0};

char l70Data1;
char gnss_str[100] = {0};
extern l70Data_t l70DataAfterHadler;

/************************************* SDCARD*******************/
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
char SD_read[256];

/**************************************SIM 4G***************************/
#define SIM 0
// typedef struct
// {
//   char online[2];
//   char offline[2];
//   char sleep[2];
//   char deviceID[20];
//   char license[20];
//   char name[50];
//   char phone[20];
//   char signal[20];
// } dataPubTB_t;
typedef struct
{
  char online[2];
  char offline[2];
  char sleep[2];
  char signal[5];
} stateDevice_t;
stateDevice_t stateDevice;
char Json_Data[500] = {0};
/**************************************RFID*****************************/
uint8_t CardID[5] = {0};
char Card_Startbuff[20] = {0};
char Card_Endbuff[20] = {0};
char *id_ref = "";
typedef struct
{
  char ID[20];      // id     64C43951
  char name[20];    // ABC
  char license[10]; // A1 A2...
  char phoneNum[20];
  char deviceID[20]; // plates license
} infoRFID_t;
infoRFID_t infoUser, infoUser1, infoUser2;

/***************************************TASK**************************/
enum
{
  Task_Sensors,
  Task_GNSS,
  Task_SIM,
  Task_Store,
  Task_Display,
  Task_LogoutOrRS232
};
volatile int Type_Task = Task_Display;
typedef struct
{
  volatile bool TaskSensors;
  volatile bool TaskGNSS;
  volatile bool TaskSIM;
  volatile bool TaskStore;
  volatile bool TaskDisplay;
} TaskPerform_t;

TaskPerform_t Taskperform = {
    .TaskSensors = false,
    .TaskGNSS = false,
    .TaskSIM = false,
    .TaskStore = false,
    .TaskDisplay = false,
};
#define SWITCH_CASE 1
#define IF_SIUTATION 0
enum
{
  Normal_Mode,
  IRQ_Mode
};
volatile int Task_Mode = Normal_Mode;
/*****************************************SENSOR*************************/
int ds18_temp;
float dht_temp;
float dht_hum;

typedef struct
{
  char ds18_buff[5];
  char dhtTemp_buff[50];
  char dhtHum_buff[50];
  char fuel[5];
} sensors_t;
sensors_t sensors;
/*******************************************FLASH***********************/
#define FLASH_ADDRESS_BASE 0x00000000
uint8_t flash_Readbuff[1024] = {0};
/******************************************RS232************************/
char rs232_data;
char rs232_buff[50] = {0};
int rs232_index = 0;
volatile bool rs232_flag = false;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void rs232_logdata(char *str)
{
  HAL_UART_Transmit(&huart2, (uint8_t *)str, strlen(str), 5000);
}

void buzzer(long time)
{
  HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_SET);
  HAL_Delay(time);
  HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);
}

void set_alarm(int intervalHour)
{
  RTC_AlarmTypeDef sAlarm = {0};
  sAlarm.AlarmTime.Hours = current_sTime.Hours + intervalHour;
  sAlarm.AlarmTime.Minutes = current_sTime.Minutes;
  sAlarm.AlarmTime.Seconds = current_sTime.Seconds;
  sAlarm.Alarm = RTC_ALARM_A;
  if (HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
}

void set_timer(long time)
{
  TIM3->CNT = 60000 - time * 10;
}

void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
  // warning and SMS
  buzzer(5000);
  sim7672_sendSMS(infoUser.phoneNum, "Lai xe qua thoi gian cho phep, xin dung lai");
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  // for SIM
  if (huart == &huart1)
  {
    sim7672_callback();
  }
  // for GPS
  if (huart == &huart4)
  {
    l70_callback();
  }
  // for RS232
  if (huart == &huart2)
  {
    Type_Task = Task_LogoutOrRS232;
    rs232_buff[rs232_index] = rs232_data;
    if (rs232_buff[rs232_index] == '\n')
    {
      rs232_flag = true;
    }
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
    if (rc522_check(CardID) == RC522_OK && waiting4LogOut == true)
    {
      waiting4LogOut = false;
      Type_Task = Task_LogoutOrRS232;
      Logout = true;
    }
  }
  if (htim == &htim3)
  {
    if (Type_Task == Task_Sensors)
    {
      Taskperform.TaskSensors = false;
      Type_Task = Task_GNSS;
      set_timer(500);
      return;
    }
    if (Type_Task == Task_GNSS)
    {
      Taskperform.TaskGNSS = false;
      Type_Task = Task_SIM;
      set_timer(1500);
      return;
    }
    if (Type_Task == Task_SIM)
    {
      Taskperform.TaskSIM = false;
      Type_Task = Task_Store;
      set_timer(500);
      return;
    }
    if (Type_Task == Task_Store)
    {
      Taskperform.TaskStore = false;
      Type_Task = Task_Display;
      set_timer(500);
      return;
    }
    if (Type_Task == Task_Display)
    {
      Taskperform.TaskDisplay = false;
      Type_Task = Task_Sensors;
      set_timer(1000);
      return;
    }
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
  // init all
  rc522_init();
  log_data("RFID init successfull\n");
  HAL_Delay(10);

  l70_init();
  log_data("GNSS init successfull\n");
  HAL_Delay(500);

  // sim7672_pwrOn();
  // HAL_Delay(4000);
  //  sim7672_init();
  //  sim7672_check_signalStrength();
  //  mqtt_connectServer();
  //  HAL_Delay(100);

  // w25q32_init();
  // log_data("FLASH init successfull\n");

  SSD1306_Init();

  SSD1306_Clear();
  SSD1306_DrawBitmap(0, 0, bitmap, 128, 64, 1);
  SSD1306_GotoXY(75, 2);
  SSD1306_Puts("2:53", &Font_7x10, 1);
  SSD1306_GotoXY(30, 16);
  SSD1306_Puts("1/1/2023", &Font_7x10, 1);
  SSD1306_GotoXY(1, 28);
  SSD1306_Puts("Lon:            E", &Font_7x10, 1);
  SSD1306_GotoXY(1, 40);
  SSD1306_Puts("Lat:           N", &Font_7x10, 1);
  SSD1306_GotoXY(1, 52);
  SSD1306_Puts("Speed:      m/s", &Font_7x10, 1);
  SSD1306_UpdateScreen();

  fresult = f_mount(&fs, "/", 1);
  if (fresult != FR_OK)
    log_data("ERROR!!! in mounting SD CARD...\n\n");
  else
    log_data("SD CARD mounted successfully...\n\n");
  // Read data FLASH
  strcpy(stateDevice.online, "1");
  strcpy(stateDevice.offline, "0");
  strcpy(stateDevice.sleep, "0");
  strcpy(stateDevice.signal, "OK");

  strcpy(infoUser1.ID, "64C43951");
  strcpy(infoUser1.name, "Nguyen_Huy_Hoang");
  strcpy(infoUser1.phoneNum, "VIE_0987035442");
  strcpy(infoUser1.license, "30N-1234");

  strcpy(infoUser2.ID, "1243");
  strcpy(infoUser2.name, "Nguyen_Van_A");
  strcpy(infoUser2.phoneNum, "VIE_0123562571");
  strcpy(infoUser2.license, "30N-3611");

  strcpy(infoUser.deviceID, "GSHT_ID01");
  // READ TAG RFID
  while (strcmp(infoUser1.ID, Card_Startbuff) != 0 || strcmp(infoUser2.ID, Card_Startbuff) != 0)
  {
    // log_data("wait\n");
    if (rc522_check(CardID) == RC522_OK)
    {
      sprintf(Card_Startbuff, "%02X%02X%02X%02X", CardID[0], CardID[1], CardID[2], CardID[3]);
      log_data(Card_Startbuff);
      if (strcmp(infoUser1.ID, Card_Startbuff) != 0)
      {
        strcpy(infoUser.ID, infoUser1.ID);
        strcpy(infoUser.name, infoUser1.name);
        strcpy(infoUser.phoneNum, infoUser1.phoneNum);
        strcpy(infoUser.license, infoUser1.license);
        buzzer(200);
        HAL_Delay(200);
        buzzer(200);
        log_data("ID1 TAG invalid\n");
      }
      if (strcmp(infoUser2.ID, Card_Startbuff) != 0)
      {
        strcpy(infoUser.ID, infoUser2.ID);
        strcpy(infoUser.name, infoUser2.name);
        strcpy(infoUser.phoneNum, infoUser2.phoneNum);
        strcpy(infoUser.license, infoUser2.license);
        buzzer(200);
        HAL_Delay(200);
        buzzer(200);
        log_data("ID2 TAG invalid\n");
      }
      else
      {
        waiting4LogOut = true;
        buzzer(200);
        log_data("ID TAG valid\n");
        break;
      }
    }
  }
  // collect time at having RFID TAG
  HAL_RTC_GetTime(&hrtc, &current_sTime, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc, &current_sDate, RTC_FORMAT_BIN);
  set_alarm(4);
  sprintf(time_buff, "%d:%d:%d", current_sTime.Hours, current_sTime.Minutes, current_sTime.Seconds);
  log_data(time_buff);
  log_data("\n");
  sprintf(date_buff, "%d/%d/%d", current_sDate.Date, current_sDate.Month, current_sDate.Year);
  log_data(date_buff);
  log_data("\n");
  HAL_RTC_GetTime(&hrtc, &last_sTime, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc, &last_sDate, RTC_FORMAT_BIN);
  totalMinute_OperateTime = (60 * last_sTime.Hours + last_sTime.Minutes) - (60 * current_sTime.Hours + current_sTime.Minutes);
  time_hour = totalMinute_OperateTime / 60;
  time_minute = totalMinute_OperateTime - time_hour * 60;
  sprintf(totalMinute_OperateTime_buff, "%dh%d", time_hour, time_minute);
  // wait for gps
  do
  {
    GPSResponse = l70_receiveGPS();
    l70_handleGPS(GPSResponse);
    HAL_Delay(1000);
  } while (strstr(l70DataAfterHadler.latitude, ".000000") != NULL && strstr(l70DataAfterHadler.longitude, ".000000") != NULL);
  log_data("\nGSHT Start\n");
  // Check ID tag in Flash

  set_timer(1000);
  HAL_TIM_Base_Start_IT(&htim3); // Task
  HAL_TIM_Base_Start(&htim1);    // Sensors
  HAL_TIM_Base_Start_IT(&htim2); // RFID
  // set_timer(1000);
  HAL_UART_Receive_IT(&huart2, (uint8_t *)&rs232_data, 1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
#if SWITCH_CASE
    switch (Type_Task)
    {
    case Task_Sensors:
      if (!Taskperform.TaskSensors)
      {
        log_data("TAKS SENSORS\n");
        Taskperform.TaskSensors = true;
        ds18_temp = (int)DS18B20_Result();
        // DHT11_Result(dht_temp, dht_hum);
        sprintf(sensors.ds18_buff, "%d", ds18_temp);
        // ftoa(ds18_temp, ds18_buff, 1);
        //  w25q32_pageWrite(0x00000000,ds18_buff);
        //  ftoa(dht_temp, dhtTemp_buff, 1);
        //  ftoa(dht_hum, dhtHum_buff, 1);
        strcpy(sensors.dhtTemp_buff, "20");
        strcpy(sensors.dhtHum_buff, "80");
        strcpy(sensors.fuel, "80");
        log_data("Nhiet do thiet bi: ");
        log_data(sensors.ds18_buff);
        log_data("\n");
      }
      break;
    case Task_GNSS:
      if (!Taskperform.TaskGNSS)
      {
        log_data("TAKS GNSS\n");
        Taskperform.TaskGNSS = true;
        GPSResponse = l70_receiveGPS();
        l70_handleGPS(GPSResponse);
      }
      break;
    case Task_SIM:
      if (!Taskperform.TaskSIM)
      {
        log_data("TAKS SIM\n");
        Taskperform.TaskSIM = true;
        memset(Json_Data,'\0',sizeof(Json_Data));
        sprintf(Json_Data, "{\"online\":%s,\"offline\":%s,\"sleep\":%s,\"deviceID\":%s,\"license\":%s,\"name\":%s,\"phone\":%s,\"signal\":%s,\"latitude\":%s,\"longitude\":%s,\"ds18b20\":%s,\"dhttemp\":%s,\"dhthum\":%s,\"fuel\":%s}",
                stateDevice.online, stateDevice.offline, stateDevice.sleep, infoUser.deviceID, infoUser.license, infoUser.name, infoUser.phoneNum,
                stateDevice.signal, l70DataAfterHadler.latitude, l70DataAfterHadler.longitude, sensors.ds18_buff, sensors.dhtTemp_buff,
                sensors.dhtHum_buff, sensors.fuel);
        mqtt_pub(TOPIC, Json_Data);
        numCheck_time++;
        if (numCheck_time == 10)
        {
          numCheck_time = 0;
          HAL_RTC_GetTime(&hrtc, &last_sTime, RTC_FORMAT_BIN);
          HAL_RTC_GetDate(&hrtc, &last_sDate, RTC_FORMAT_BIN);
          totalMinute_OperateTime = (60 * last_sTime.Hours + last_sTime.Minutes) - (60 * current_sTime.Hours + current_sTime.Minutes);
          time_hour = totalMinute_OperateTime / 60;
          time_minute = totalMinute_OperateTime - time_hour * 60;
          sprintf(totalMinute_OperateTime_buff, "%dh%d", time_hour, time_minute);
          log_data(totalMinute_OperateTime_buff);
        }
      }
      break;
    case Task_Store:
      if (!Taskperform.TaskStore)
      {
        log_data("TASK STORE\n");
        Taskperform.TaskStore = true;

        fresult = f_open(&fil, "GSHT1.txt", FA_WRITE);
        if (fresult != FR_OK)
          log_data("open error...\n\n");
        else
          log_data("open successfully...\n\n");

        f_lseek(&fil, f_size(&fil));

        f_puts(l70DataAfterHadler.utcTime, &fil);
        f_puts("\t", &fil);
        f_puts(Card_Startbuff, &fil);
        f_puts("\t", &fil);
        f_puts(l70DataAfterHadler.latitude, &fil);
        f_puts("\t", &fil);
        f_puts(l70DataAfterHadler.longitude, &fil);
        f_puts("\t", &fil);
        f_puts(l70DataAfterHadler.Speed, &fil);
        f_puts("\t", &fil);
        f_puts(sensors.ds18_buff, &fil);
        f_puts("\n", &fil);

        fresult = f_close(&fil);
      }
      break;
    case Task_Display:
      if (!Taskperform.TaskDisplay)
      {
        log_data("TASK DISPLAY\n");
        Taskperform.TaskDisplay = true;
        SSD1306_GotoXY(75, 2);
        SSD1306_Puts(l70DataAfterHadler.utcTime, &Font_7x10, 1);
        SSD1306_GotoXY(30, 16);
        SSD1306_Puts(l70DataAfterHadler.Date, &Font_7x10, 1);
        SSD1306_GotoXY(25, 28);
        SSD1306_Puts(l70DataAfterHadler.longitude, &Font_7x10, 1);
        SSD1306_GotoXY(25, 40);
        SSD1306_Puts(l70DataAfterHadler.latitude, &Font_7x10, 1);
        SSD1306_GotoXY(35, 52);
        SSD1306_Puts(l70DataAfterHadler.Speed, &Font_7x10, 1);
        SSD1306_UpdateScreen();
      }
      break;
    case Task_LogoutOrRS232:
      // log_data("Task Logout or RS232\n");
      if (Logout)
      {
        Logout = false;
        sprintf(Card_Endbuff, "%02X%02X%02X%02X", CardID[0], CardID[1], CardID[2], CardID[3]);
        if (strcmp(Card_Startbuff, Card_Endbuff) != 0)
        {
          log_data("Tag invalid\n");
        }
        else
        {
          log_data("Tag valid, End of schedual\n");
          // write flash totaltime
          HAL_RTC_GetTime(&hrtc, &last_sTime, RTC_FORMAT_BIN);
          HAL_RTC_GetDate(&hrtc, &last_sDate, RTC_FORMAT_BIN);
          totalMinute_OperateTime = (60 * last_sTime.Hours + last_sTime.Minutes) - (60 * current_sTime.Hours + current_sTime.Minutes);
          set_alarm(10);
        }
        // sleep
        HAL_Delay(1000);
        log_data("Device start sleep\n");
      }
      if (rs232_flag)
      {
        rs232_flag = false;
        if (strstr(rs232_buff, "Trich xuat du lieu\n") != NULL)
        {
          fresult = f_open(&fil, "GSHT1.txt", FA_READ);
          if (fresult != FR_OK)
            log_data("open error...\n\n");
          else
            log_data("open successfully...\n\n");
          while (f_gets(SD_read, sizeof(SD_read), &fil) != NULL)
          {
            rs232_logdata(SD_read);
          }
        }
      }
      break;
    }
    HAL_Delay(100);
#endif
    // HAL_Delay(100);
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC | RCC_PERIPHCLK_ADC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
#if IF_SIUTATION
if (Type_Task == Task_Sensors && !Taskperform.TaskSensors)
{
  log_data("TAKS SENSORS\n");
  Taskperform.TaskSensors = true;
  log_data("TAKS SENSORS\n");
  Taskperform.TaskSensors = true;
  ds18_temp = (int)DS18B20_Result();
  // DHT11_Result(dht_temp, dht_hum);
  sprintf(ds18_buff, "%d", ds18_temp);
  // ftoa(ds18_temp, ds18_buff, 1);
  //  w25q32_pageWrite(0x00000000,ds18_buff);
  //  ftoa(dht_temp, dhtTemp_buff, 1);
  //  ftoa(dht_hum, dhtHum_buff, 1);
  log_data("Nhiet do thiet bi: ");
  log_data(ds18_buff);
  log_data("\n");
}
if (Type_Task == Task_GNSS && !Taskperform.TaskGNSS)
{
  log_data("TAKS GNSS\n");
  Taskperform.TaskGNSS = true;
  GPSResponse = l70_receiveGPS();
  l70_handleGPS(GPSResponse);
}
if (Type_Task == Task_SIM && !Taskperform.TaskSIM)
{

  log_data("TAKS SIM\n");
  Taskperform.TaskSIM = true;
  // mqtt_pub(TOPIC_TEST, Card_Startbuff);
  // mqtt_pub(TOPIC_TEST, ds18_buff);
  // // mqtt_pub(TOPIC_TEST, dhtHum_buff);
  // // mqtt_pub(TOPIC_TEST, dhtTemp_buff);
  // mqtt_pub(TOPIC_TEST, l70DataAfterHadler.longitude);
  // mqtt_pub(TOPIC_TEST, l70DataAfterHadler.latitude);
  // numCheck_time++;
  // if (numCheck_time == 10)
  // {
  //   numCheck_time = 0;
  //   HAL_RTC_GetTime(&hrtc, &last_sTime, RTC_FORMAT_BIN);
  //   HAL_RTC_GetDate(&hrtc, &last_sDate, RTC_FORMAT_BIN);
  //   totalMinute_OperateTime = (60 * last_sTime.Hours + last_sTime.Minutes) - (60 * current_sTime.Hours + current_sTime.Minutes);
  //   sprintf(totalMinute_OperateTime_buff, "totalTime: %d", totalMinute_OperateTime);
  //   mqtt_pub(TOPIC_TEST, totalMinute_OperateTime_buff);
  // }
}
if (Type_Task == Task_Store && !Taskperform.TaskStore)
{
  log_data("TASK STORE\n");
  Taskperform.TaskStore = true;

  fresult = f_open(&fil, "GSHT1.txt", FA_WRITE);
  if (fresult != FR_OK)
    log_data("open error...\n\n");
  else
    log_data("open successfully...\n\n");

  f_lseek(&fil, f_size(&fil));

  f_puts(l70DataAfterHadler.utcTime, &fil);
  f_puts("\t", &fil);
  f_puts(Card_Startbuff, &fil);
  f_puts("\t", &fil);
  f_puts(l70DataAfterHadler.latitude, &fil);
  f_puts("\t", &fil);
  f_puts(l70DataAfterHadler.longitude, &fil);
  f_puts("\t", &fil);
  f_puts(l70DataAfterHadler.Speed, &fil);
  f_puts("\t", &fil);
  f_puts(ds18_buff, &fil);
  f_puts("\n", &fil);

  fresult = f_close(&fil);
}
if (Type_Task == Task_Display && !Taskperform.TaskDisplay)
{
  log_data("TASK DISPLAY\n");
  Taskperform.TaskDisplay = true;
}
HAL_Delay(100);
#endif
// #if SIM
//   // sim7672_pwrOn();
//   // HAL_Delay(5000);
//   sim7672_init();
//   sim7672_check_signalStrength();
//   HAL_Delay(500);
//   // mqtt_disconnectServer();
//   HAL_Delay(1000);
//   mqtt_connectServer();
//   mqtt_pub("test1z", "123456789");
//   HAL_Delay(500);
//   mqtt_sub("test1z");
// #endif

// #if GPS
//   sim7672_pwrOff();
//   l70_init();
//   HAL_Delay(1000);
// #endif

// #if SDCard

//   fresult = f_mount(&fs, "/", 1);
//   if (fresult != FR_OK)
//     log_data("ERROR!!! in mounting SD CARD...\n\n");
//   else
//     log_data("*******************\nSD CARD mounted successfully...\n\n");

//   /* ********************************Create second file with read write access and open it */
//   fresult = f_open(&fil, "file2.txt", FA_WRITE);
//   if (fresult != FR_OK)
//     log_data("open error...\n\n");
//   else
//     log_data("open successfully...\n\n");
//   // f_lseek(&fil, f_size(&fil));
//   f_puts("This data is from the FILE1.txt. And it was written using ...f_puts... \nthis full function", &fil);
//   // f_puts("\nThis is updated data and it should be in the end",&fil);
//   fresult = f_close(&fil);
//   if (fresult != FR_OK)
//     log_data("write error...\n\n");
//   else
//     log_data("write successfully...\n\n");
//   log_data("File2.txt created and data is written\n");
//   // clearing buffer to show that result obtained is from the file

//   /* ***********************************Open second file to read */
//   fresult = f_open(&fil, "file2.txt", FA_READ);
//   if (fresult != FR_OK)
//     log_data("open error...\n\n");
//   else
//     log_data("open successfully...\n\n");

//   /* Read data from the file
//    * Please see the function details for the arguments */
//   // f_gets(buffer, f_size(&fil), &fil);
//   while (f_gets(buffer, sizeof(buffer), &fil) != NULL)
//   {
//     // Xử lý dữ liệu theo nhu cầu của bạn, ví dụ: gửi qua UART
//     log_data(buffer);
//   }
//   // f_read (&fil, buffer, f_size(&fil), &br);
//   log_data("data: ");
//   log_data(buffer);
//   log_data("\n\n");

//   /* Close file */
//   f_close(&fil);
// // HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_SET);
// // HAL_Delay(1000);
// // HAL_GPIO_WritePin(BUZZER_GPIO_Port,BUZZER_Pin,GPIO_PIN_RESET);
// #endif
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

#ifdef USE_FULL_ASSERT
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
