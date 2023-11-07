/*
 * common.c
 *
 *  Created on: Feb 8, 2023
 *      Author: dung
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/unistd.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>

#include "main.h"

#include "common.h"

#define USE_UART
#define USE_FREERTOS



// #include "task.h"
// #include "cmsis_os.h"

static SemaphoreHandle_t log_semaphore;

void Log_Init(void)
{
    log_semaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(log_semaphore);
}

#else
void Log_Init(void) {}
#endif

#ifdef USE_USB
#include "usbd_cdc_if.h"
extern USBD_HandleTypeDef hUsbDeviceFS;
#endif

#ifdef USE_UART
#include "usart.h"
#define UART    (&huart1)
#endif


void LOG(const char *TAG, char *data)
{
#ifdef USE_UART
	// HAL_UART_Transmit(UART, (uint8_t*)TAG, strlen(TAG), 100);
    // HAL_UART_Transmit(UART, (uint8_t*)" - ", 3, 100);
    // HAL_UART_Transmit(UART, (uint8_t*)data, strlen(data), 100);

    logPC("%s - %s", TAG, data);
#endif

#ifdef USE_USB


#endif /* USE_USB */
}

#if DEBUG == 1

static char string[200];

/**
 * @brief 
 * 
 * @param fmt 
 * @param argp 
 */
void vprint(const char *fmt, va_list argp)
{
    memset(string, '\0', sizeof(string));
    if(0 < vsprintf(string, fmt, argp)) // build string
    {
#ifdef USE_UART
        HAL_UART_Transmit(UART, (uint8_t*)string, strlen(string), 100);
#endif
#ifdef USE_USB
        CDC_Transmit_FS(string, strlen(string));
#endif /* USE_USB */
    }
}

/**
 * @brief custom printf() function
 * 
 * @param fmt 
 * @param ... 
 */
void logPC(const char *fmt, ...)
{
#ifdef USE_FREERTOS
    xSemaphoreTake(log_semaphore, portMAX_DELAY);
#endif
    va_list argp;
    va_start(argp, fmt);
    vprint(fmt, argp);
    va_end(argp);
#ifdef USE_FREERTOS
    xSemaphoreGive(log_semaphore);
#endif
}

#else
void logPC(const char *fmt, ...)
{
    UNUSED(fmt);
}
#endif




void reverse(char* str, int len)
{
    int i = 0, j = len - 1, temp;
    while (i < j)
    {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++;
        j--;
    }
}

int intToStr(int x, char str[], int d)
{
    int i = 0;
    if (x == 0)
    {
        str[i++] = '0';
    }
    else
    {
        while (x)
        {
            str[i++] = (x % 10) + '0';
            x = x / 10;
        }
    }
    
    // If number of digits required is more, then
    // add space at the beginning
    while (i < d)
        str[i++] = ' ';

    reverse(str, i);
    str[i] = '\0';
    return i;
}

void ftoa(float n, char* res, int afterpoint)
{
    // Extract integer part
    int ipart = (int)n;
    // Extract floating part
    float fpart = n - (float)ipart;
    // convert integer part to string
    int i = intToStr(ipart, res, 0);
    // check for display option after point
    if (afterpoint != 0)
    {
        res[i] = '.'; // add dot
        fpart = fpart * pow(10, afterpoint);
        intToStr((int)fpart, res + i + 1, afterpoint);
    }
}

int intToStr0(int x, char str[], int d)
{
    int i = 0;
    if (x == 0)
    {
        str[i++] = '0';
    }
    else
    {
        while (x)
        {
            str[i++] = (x % 10) + '0';
            x = x / 10;
        }
    }
    
    // If number of digits required is more, then
    // add 0s at the beginning
    while (i < d)
        str[i++] = '0';

    reverse(str, i);
    str[i] = '\0';
    return i;
}

void ftoa0(float n, char* res, int afterpoint)
{
    // Extract integer part
    int ipart = (int)n;
    // Extract floating part
    float fpart = n - (float)ipart;
    // convert integer part to string
    int i = intToStr0(ipart, res, 0);
    // check for display option after point
    if (afterpoint != 0)
    {
        res[i] = '.'; // add dot
        fpart = fpart * pow(10, afterpoint);
        intToStr((int)fpart, res + i + 1, afterpoint);
    }
}

int random_number(int min_num, int max_num)
{
    int result = 0, low_num = 0, hi_num = 0;

    if (min_num < max_num)
    {
        low_num = min_num;
        hi_num = max_num + 1; // include max_num in output
    } else {
        low_num = max_num + 1; // include max_num in output
        hi_num = min_num;
    }

    srand(HAL_GetTick());
    result = (rand() % (hi_num - low_num)) + low_num;
    return result;
}



