#ifndef _GNSS_H__
#define _GNSS_H__

#include "main.h"
#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "debug.h"
// #include "stm32f100xe.h"

#define L70_CHECK 0
#define L70_NOT_CHECK 1
#define KNOT_TO_KM  1.852
#define GMT 7
#define SPEEDTHRESH 40.0

typedef struct
{
    char utcTime[12];
    char latitude[20];
    char longitude[20];
    char Speed[10];
    char COGdirect[10];
    char Date[10];
} l70Data_t;

typedef enum
{
    L70_START_DATA,
    L70_DATA,
    L70_END_DATA
} l70DataRecive_t;

typedef struct
{
    char utcTime[12];
    char latitude[20];
    char NorS;
    char longitude[20];
    char EorW;
    char fixStatus;
    char numberOfSatellites[2];
    char HDOP[10];
    char altitude[10];
} l70GPGGAType_t;

typedef struct
{
    char utcTime[12];
    char dataValid;
    char latitude[20];
    char NorS;
    char longitude[20];
    char EorW;
    char Speed[10];
    char COGdirect[10];
    char Date[10];
} l70GPRMCType_t;

typedef struct 
{
    int hour;
    int minute;
    int sencond;
} l70Utctime_t;

typedef struct 
{
    int day;
    int month;
    int year;
} l70Date_t;

extern UART_HandleTypeDef huart4;


void l70_callback(void);
void l70_init(void);
void l70_sleep(void);
double minuteTodegree(char *data);
void l70_handleGPS(char *GPSResponse);
char *l70_receiveGPS(void);
void l70_checkSpeed();

#endif