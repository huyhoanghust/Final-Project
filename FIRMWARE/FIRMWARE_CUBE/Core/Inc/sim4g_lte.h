#ifndef _SIM4G_LTE_H__
#define _SIM4G_LTE_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "main.h"
#include "debug.h"

extern TIM_HandleTypeDef htim3;
extern UART_HandleTypeDef huart1; 

typedef enum
{
    SIM_NOT_CHECK,
    SIM_CHECK
} simCheck_t;

typedef enum
{
    SIM_NOT_OKE,
    SIM_OKE
} simState_t;

typedef enum
{
    SIM_NOT_RX_CPLT,
    SIM_RX_CPLT
} simRxCplt_t;

void sim7672_pwrkey(void);
void sim7672_init(void);
simState_t sim7672_send_command(char *simCommand, char *trueResponse, int checkOrNot);
void sim7672_callback(void);
void sim7672_timerCallback(void);
void sim7672_errorHandle(void);


#endif