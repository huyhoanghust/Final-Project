/**
 * @file sim.h
 * @author Vento (zseefvhu12345@gmail.com)
 * @brief 
 * @version 1.0
 * @date 24-07_2023
 * 
 * @copyright Copyright (c) 2023
 * 
 */


#ifndef _SIM_H_
#define _SIM_H_

#include "main.h"
#include "usart.h"
#include "stdbool.h"

#define UART_SIM    &huart1

#define CMD_DELAY_LONG          10000
#define CMD_DELAY_MEDIUM        5000
#define CMD_DELAY_SHORT         2000
#define CMD_DELAY_VERYSHORT     500


#define PWRKEY_PORT     SIM_PWRKEY_GPIO_Port
#define PWRKEY_PIN      SIM_PWRKEY_Pin
#define PWR_EN_PORT     SIM_PWR_EN_GPIO_Port
#define PWR_EN_PIN      SIM_PWR_EN_Pin
#define RESET_PORT      SIM_RST_GPIO_Port
#define RESET_PIN       SIM_RST_Pin

//------------------------------------------------------------------------------------------------------

typedef struct 
{
    uint8_t Octec1;
    uint8_t Octec2;
    uint8_t Octec3;
    uint8_t Octec4;
} SIM_IP_t;

typedef enum
{
    SIM_SIMCARD_OK,
    SIM_SIMCARD_NOK,
    SIM_GPRS_OK,
    SIM_GPRS_NOK
} SIM_ST_t;

typedef struct 
{
    char *apn;
    char *apn_user;
    char *apn_pass;
    uint8_t signalQuality;
    uint8_t SIMCardStatusCode;
    uint32_t Balance;
    SIM_IP_t IP;
    SIM_ST_t status;
} SIM_t;

typedef void (*MQTT_Callback_func_p)(const char*);

//------------------------------------------------------------------------------------------------------

extern SIM_t SIM;
extern volatile bool isWaiting4Response;
extern uint8_t responseBuffer[200];

//------------------------------------------------------------------------------------------------------

bool SIM_Init(void);

bool SIM_Deinit(void);

void SIM_Reset(void);

void SIM_GetResponse(const char* response);

void SIM_sendATCommand(char* command);

void SIM_sendATCommand_withLength(char* command, uint32_t len);

bool SIM_sendATCommandResponse(char* command, char* response, uint32_t waitms);

bool SIM_checkSIMCard(void);

void SIM_sendSMS(char* number, char* message);

void SIM_call(char* number);

bool SIM_startGPRS(void);

bool SIM_getIP(void);

uint8_t SIM_checkSignalStrength(void);


void setMQTT_Callback(MQTT_Callback_func_p func);

#endif /* _SIM_H_ */

