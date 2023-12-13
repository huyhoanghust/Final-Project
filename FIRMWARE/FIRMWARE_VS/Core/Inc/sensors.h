#ifndef __SENSORS_H_
#define __SENSORS_H_

//#include "stm32f1xx_hal.h"
#include <stdio.h>
#include "tim.h"
#include "adc.h"
#include "debug.h"

#define INIT_SUCCESS    1
#define INIT_FAIL    1
/*************for DS18B2220**************/
uint8_t DS18B20_Start (void);
void DS18B20_Write (uint8_t data);
uint8_t DS18B20_Read (void);
float DS18B20_Result(void);

/*************for DHT11**************/
void DHT11_Start(void);
uint8_t DHT11_Check_Response(void);
uint8_t DHT11_Read(void);
float DHT11_Result(float Temperature, float Humidity);

/************for LEVEL***************/
//4095 -> 0cm
float Level_Result();

/************for ADC ACCQUI***************/
//4095-> 3.3V
float Level_Result();


#endif