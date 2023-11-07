/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define NPN_DATA_IN_Pin GPIO_PIN_0
#define NPN_DATA_IN_GPIO_Port GPIOC
#define ADC_VIN_ACCQI_Pin GPIO_PIN_1
#define ADC_VIN_ACCQI_GPIO_Port GPIOC
#define LEVEL_SENSOR_Pin GPIO_PIN_2
#define LEVEL_SENSOR_GPIO_Port GPIOC
#define DS18B20_SENSOR_Pin GPIO_PIN_3
#define DS18B20_SENSOR_GPIO_Port GPIOC
#define RS232_TX_Pin GPIO_PIN_2
#define RS232_TX_GPIO_Port GPIOA
#define RS232_RX_Pin GPIO_PIN_3
#define RS232_RX_GPIO_Port GPIOA
#define RFID_SS_Pin GPIO_PIN_4
#define RFID_SS_GPIO_Port GPIOA
#define RFID_CLK_Pin GPIO_PIN_5
#define RFID_CLK_GPIO_Port GPIOA
#define RFID_MISO_Pin GPIO_PIN_6
#define RFID_MISO_GPIO_Port GPIOA
#define RFID_MOSI_Pin GPIO_PIN_7
#define RFID_MOSI_GPIO_Port GPIOA
#define DHT11_SENSOR_Pin GPIO_PIN_4
#define DHT11_SENSOR_GPIO_Port GPIOC
#define SDCARD_SS_Pin GPIO_PIN_1
#define SDCARD_SS_GPIO_Port GPIOB
#define FLASH_SS_Pin GPIO_PIN_12
#define FLASH_SS_GPIO_Port GPIOB
#define FLASH_CLK_Pin GPIO_PIN_13
#define FLASH_CLK_GPIO_Port GPIOB
#define FLASH_MISO_Pin GPIO_PIN_14
#define FLASH_MISO_GPIO_Port GPIOB
#define FLASH_MOSI_Pin GPIO_PIN_15
#define FLASH_MOSI_GPIO_Port GPIOB
#define GNSS_CTRL_PWR_Pin GPIO_PIN_8
#define GNSS_CTRL_PWR_GPIO_Port GPIOC
#define GNSS_RESET_Pin GPIO_PIN_9
#define GNSS_RESET_GPIO_Port GPIOC
#define SIM_STATUS_Pin GPIO_PIN_8
#define SIM_STATUS_GPIO_Port GPIOA
#define SIM_TX_Pin GPIO_PIN_9
#define SIM_TX_GPIO_Port GPIOA
#define SIM_RX_Pin GPIO_PIN_10
#define SIM_RX_GPIO_Port GPIOA
#define SIM_CTRL_PWR_Pin GPIO_PIN_11
#define SIM_CTRL_PWR_GPIO_Port GPIOA
#define SIM_PWRKEY_Pin GPIO_PIN_12
#define SIM_PWRKEY_GPIO_Port GPIOA
#define SIM_RESET_Pin GPIO_PIN_15
#define SIM_RESET_GPIO_Port GPIOA
#define GNSS_TX_Pin GPIO_PIN_10
#define GNSS_TX_GPIO_Port GPIOC
#define GNSS_RX_Pin GPIO_PIN_11
#define GNSS_RX_GPIO_Port GPIOC
#define DEBUG_TX_Pin GPIO_PIN_12
#define DEBUG_TX_GPIO_Port GPIOC
#define DEBUG_RX_Pin GPIO_PIN_2
#define DEBUG_RX_GPIO_Port GPIOD
#define SDCARD_CLK_Pin GPIO_PIN_3
#define SDCARD_CLK_GPIO_Port GPIOB
#define SDCARD_MISO_Pin GPIO_PIN_4
#define SDCARD_MISO_GPIO_Port GPIOB
#define SDCARD_MOSI_Pin GPIO_PIN_5
#define SDCARD_MOSI_GPIO_Port GPIOB
#define OLED_SCL_Pin GPIO_PIN_6
#define OLED_SCL_GPIO_Port GPIOB
#define OLED_SDA_Pin GPIO_PIN_7
#define OLED_SDA_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
