#include "debug.h"

void log_data(char *string)
{
  HAL_UART_Transmit(&huart5, (uint8_t *)string, strlen(string), 1000);
}