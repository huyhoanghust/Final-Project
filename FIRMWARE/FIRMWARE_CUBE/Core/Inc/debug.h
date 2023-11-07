#ifndef _DEBUG_H__
#define _DEBUG_H__

#include "main.h"
#include <string.h>

extern UART_HandleTypeDef huart5;

void log_data(char *string);

#endif