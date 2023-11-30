#ifndef _SIM4G_LTE_H__
#define _SIM4G_LTE_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "main.h"
#include "debug.h"
#include "stdbool.h"

#define TIME_VERY_SHORT 200
#define TIME_SHORT      500
#define TIME_MEDIUM     1000
#define TIME_LONG       1500

extern UART_HandleTypeDef huart1;

typedef enum
{
    SIM_NOT_CHECK,
    SIM_CHECK
} simCheck_t;

typedef enum
{
    SIM_FALSE,
    SIM_TRUE
} simState_t;

typedef enum
{
    SIM_NOT_RX_CPLT,
    SIM_RX_CPLT
} simRxCplt_t;

typedef struct
{
    char *apn;
    char *apn_user;
    char *apn_pass;
    uint8_t signalQuality;
    uint8_t SIMCardStatusCode;
} simInfo_t;

void sim7672_reset(void);
void sim7672_pwrOff(void);
void sim7672_pwrOn(void);
void sim7672_init(void);
void sim7672_sendcmd_notresp(char *cmd);
simState_t sim7672_send_command(char *simCommand, char *trueResponse, int waitms);
void sim7672_callback(void);
void sim7672_start_LTE(void);
uint8_t sim7672_check_signalStrength(void);
void sim7672_errorHandle(void);
void sim7672_checkSIMCard();

#endif