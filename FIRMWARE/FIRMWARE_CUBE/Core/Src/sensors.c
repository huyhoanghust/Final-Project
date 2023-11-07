#include "sensors.h"



enum State
{
	Stop,
	Move
};

int detect;

void delay(uint16_t time)
{
	/* change your code here for the delay in microseconds */
	__HAL_TIM_SET_COUNTER(&htim1, 0);
	while ((__HAL_TIM_GET_COUNTER(&htim1)) < time)
		;
}

void Set_Pin_Output(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = GPIO_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

void Set_Pin_Input(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = GPIO_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

/***************************DS18B20***********************/
uint8_t DS18B20_Start(void)
{
	uint8_t Response = 0;
	Set_Pin_Output(DS18B20_SENSOR_GPIO_Port, DS18B20_SENSOR_Pin);		// set the pin as output
	HAL_GPIO_WritePin(DS18B20_SENSOR_GPIO_Port, DS18B20_SENSOR_Pin, 0); // pull the pin low
	delay(480);															// delay according to datasheet

	Set_Pin_Input(DS18B20_SENSOR_GPIO_Port, DS18B20_SENSOR_Pin); // set the pin as input
	delay(80);													 // delay according to datasheet

	if (!(HAL_GPIO_ReadPin(DS18B20_SENSOR_GPIO_Port, DS18B20_SENSOR_Pin)))
		Response = 1; // if the pin is low i.e the presence pulse is detected
	else
		Response = -1;

	delay(400); // 480 us delay totally.

	return Response;
}

void DS18B20_Write(uint8_t data)
{
	Set_Pin_Output(DS18B20_SENSOR_GPIO_Port, DS18B20_SENSOR_Pin); // set as output

	for (int i = 0; i < 8; i++)
	{

		if ((data & (1 << i)) != 0) // if the bit is high
		{
			// write 1

			Set_Pin_Output(DS18B20_SENSOR_GPIO_Port, DS18B20_SENSOR_Pin);		// set as output
			HAL_GPIO_WritePin(DS18B20_SENSOR_GPIO_Port, DS18B20_SENSOR_Pin, 0); // pull the pin LOW
			delay(1);															// wait for 1 us

			Set_Pin_Input(DS18B20_SENSOR_GPIO_Port, DS18B20_SENSOR_Pin); // set as input
			delay(50);													 // wait for 60 us
		}

		else // if the bit is low
		{
			// write 0

			Set_Pin_Output(DS18B20_SENSOR_GPIO_Port, DS18B20_SENSOR_Pin);
			HAL_GPIO_WritePin(DS18B20_SENSOR_GPIO_Port, DS18B20_SENSOR_Pin, 0); // pull the pin LOW
			delay(50);															// wait for 60 us

			Set_Pin_Input(DS18B20_SENSOR_GPIO_Port, DS18B20_SENSOR_Pin);
		}
	}
}

uint8_t DS18B20_Read(void)
{
	uint8_t value = 0;

	Set_Pin_Input(DS18B20_SENSOR_GPIO_Port, DS18B20_SENSOR_Pin);

	for (int i = 0; i < 8; i++)
	{
		Set_Pin_Output(DS18B20_SENSOR_GPIO_Port, DS18B20_SENSOR_Pin); // set as output

		HAL_GPIO_WritePin(DS18B20_SENSOR_GPIO_Port, DS18B20_SENSOR_Pin, 0); // pull the data pin LOW
		delay(1);															// wait for > 1us

		Set_Pin_Input(DS18B20_SENSOR_GPIO_Port, DS18B20_SENSOR_Pin);		// set as input
		if (HAL_GPIO_ReadPin(DS18B20_SENSOR_GPIO_Port, DS18B20_SENSOR_Pin)) // if the pin is HIGH
		{
			value |= 1 << i; // read = 1
		}
		delay(50); // wait for 60 us
	}
	return value;
}

float DS18B20_Result(void)
{
	uint8_t Presence = 0;
	uint8_t Temp_byte1, Temp_byte2;
	uint16_t TEMP;
	float Temperature;
	Presence = DS18B20_Start();
	HAL_Delay(1);
	DS18B20_Write(0xCC); // skip ROM
	DS18B20_Write(0x44); // convert t
	HAL_Delay(800);

	Presence = DS18B20_Start();
	HAL_Delay(1);
	DS18B20_Write(0xCC); // skip ROM
	DS18B20_Write(0xBE); // Read Scratch-pad

	Temp_byte1 = DS18B20_Read();
	Temp_byte2 = DS18B20_Read();
	TEMP = (Temp_byte2 << 8) | Temp_byte1;
	Temperature = (float)TEMP / 16;
	return Temperature;
}

/*****************************DHT11*************************/
void DHT11_Start(void)
{
	Set_Pin_Output(DHT11_SENSOR_GPIO_Port, DHT11_SENSOR_Pin);		// set the pin as output
	HAL_GPIO_WritePin(DHT11_SENSOR_GPIO_Port, DHT11_SENSOR_Pin, 0); // pull the pin low
	delay(18000);													// wait for 18ms		1.2ms for DHT22 10ms
	HAL_GPIO_WritePin(DHT11_SENSOR_GPIO_Port, DHT11_SENSOR_Pin, 1); // pull the pin high
	delay(20);														// wait for 20us
	Set_Pin_Input(DHT11_SENSOR_GPIO_Port, DHT11_SENSOR_Pin);		// set as input
}

uint8_t DHT11_Check_Response(void)
{
	uint8_t Response = 0;
	delay(40);
	if (!(HAL_GPIO_ReadPin(DHT11_SENSOR_GPIO_Port, DHT11_SENSOR_Pin))) // if the pin is low
	{
		delay(80);
		if ((HAL_GPIO_ReadPin(DHT11_SENSOR_GPIO_Port, DHT11_SENSOR_Pin)))
			Response = 1; // if the pin is high, response is ok
		else
			Response = -1; // 255
	}
	while ((HAL_GPIO_ReadPin(DHT11_SENSOR_GPIO_Port, DHT11_SENSOR_Pin)))
		; // wait for the pin to go low

	return Response;
}

uint8_t DHT11_Read(void)
{
	uint8_t i, j;
	for (j = 0; j < 8; j++)
	{
		while (!(HAL_GPIO_ReadPin(DHT11_SENSOR_GPIO_Port, DHT11_SENSOR_Pin)));	// wait for the pin to go high
		delay(40);														   // wait for 40 us
		if (!(HAL_GPIO_ReadPin(DHT11_SENSOR_GPIO_Port, DHT11_SENSOR_Pin))) // if the pin is low
		{
			i &= ~(1 << (7 - j)); // write 0
		}
		else
			i |= (1 << (7 - j)); // if the pin is high, write 1
		while ((HAL_GPIO_ReadPin(DHT11_SENSOR_GPIO_Port, DHT11_SENSOR_Pin)))
			; // wait for the pin to go low
	}
	return i;
}

float DHT11_Result(float* Temperature, float* Humidity)
{
	uint8_t Presence = 0;
	uint8_t Rh_byte1, Rh_byte2, Temp_byte1, Temp_byte2;
	uint16_t SUM, RH, TEMP;
	// float Temperature = 0;
	// float Humidity = 0;
	DHT11_Start();
	Presence = DHT11_Check_Response();
	Rh_byte1 = DHT11_Read();
	Rh_byte2 = DHT11_Read();
	Temp_byte1 = DHT11_Read();
	Temp_byte2 = DHT11_Read();
	SUM = DHT11_Read();

	TEMP = Temp_byte1;
	RH = Rh_byte1;

	*Temperature = (float)TEMP;
	*Humidity = (float)RH;
}

/************for LEVEL***************/
//4095 -> 0cm
float Level_Result()
{
	uint16_t value_level;
	float level;
	value_level = HAL_ADC_GetValue(&hadc2);
	level = 10.0-(10/4095)*(float)value_level;
	return level;
}

/************for ADC ACCQUI***************/
//4095-> 3.3V
float Accqui_Result()
{
	uint16_t value_accqui;
	float vol_accqi;
	value_accqui = HAL_ADC_GetValue(&hadc1);
	vol_accqi = (3.3/4095)*(float)value_accqui;
	return vol_accqi;
}

// /*************NPN cam bien Tiem can**********/
// int Vehical_State()
// {
// 	int vhc_state;
// 	detect = HAL_GPIO_ReadPin(NPN_DATA_IN_GPIO_Port, NPN_DATA_IN_Pin);
// }
















