/**
 * @file sim.c
 * @author Vento (zseefvhu12345@gmail.com)
 * @brief 
 * @version 1.0
 * @date 24-07_2023
 * 
 * @copyright Copyright (c) 2023
 * 
 */


#include "sim.h"
#include "stdio.h"
#include "common.h"
#include "string.h"

#if FREERTOS == 1
#include "cmsis_os2.h"
#endif


SIM_t SIM = {"m3-world", "mms", "mms", 0, 0, 0, {0, 0, 0, 0}, SIM_SIMCARD_NOK};
uint8_t responseBuffer[200] = {0};
volatile bool isWaiting4Response = false;
static MQTT_Callback_func_p MQTT_cb_p = NULL;


void setMQTT_Callback(MQTT_Callback_func_p func)
{
    if (func == NULL)   return;
    MQTT_cb_p = func;
}

inline static void SIM_Delay(uint32_t delay)
{
#if FREERTOS == 1
    osDelay(delay);
#else
    HAL_Delay(delay);
#endif
}

static bool SIM_Init_Func(void)
{
    bool error = true;

    error &= SIM_sendATCommandResponse("AT+CFUN?\r\n", "1", CMD_DELAY_VERYSHORT);
    // error &= SIM_sendATCommandResponse("ATE0\r\n", "OK", CMD_DELAY_VERYSHORT);
    error &= SIM_checkSIMCard();

    return error;
}

//----------------------------------------------------------------------------------

void SIM_GetResponse(const char* response)
{
    if (MQTT_cb_p != NULL)  MQTT_cb_p(response);
    memset(responseBuffer, '\0', sizeof(responseBuffer));
    memcpy(responseBuffer, response, sizeof(responseBuffer));
    for (uint8_t idx = 0; idx < sizeof(responseBuffer)-1; idx++)
    {   
        if ((responseBuffer[idx] == '\r') && (responseBuffer[idx+1] == '\n'))
            memcpy(&responseBuffer[idx], &responseBuffer[idx+2], sizeof(responseBuffer)-idx-2);
    }

    isWaiting4Response = false;
}

inline void SIM_sendATCommand(char* command)
{
    HAL_UART_Transmit(UART_SIM, (unsigned char *)command, (uint16_t)strlen(command), 100);
}

inline void SIM_sendATCommand_withLength(char* command, uint32_t len)
{
    HAL_UART_Transmit(UART_SIM, (unsigned char *)command, len, 100);
}

bool SIM_sendATCommandResponse(char* command, char* response, uint32_t waitms)
{
    uint32_t timeOut = HAL_GetTick();
    isWaiting4Response = true;

    HAL_UART_Transmit(UART_SIM, (unsigned char *)command, (uint16_t)strlen(command), 100);

    while ((isWaiting4Response == true) && ((HAL_GetTick() - timeOut) <= waitms))
    {
        SIM_Delay(200);
    }

    if (((HAL_GetTick() - timeOut) >= waitms))      return false;

    if (strstr(responseBuffer, response) != NULL)   return true;

    return false;
}

//----------------------------------------------------------------------------------

bool SIM_Init(void)
{
    uint32_t timeout = HAL_GetTick();
    logPC("SIM - PWRDN");
    HAL_GPIO_WritePin(PWR_EN_PORT, PWR_EN_PIN, GPIO_PIN_SET);
    SIM_Delay(CMD_DELAY_MEDIUM);
    logPC("SIM - PWRUP");
    HAL_GPIO_WritePin(PWR_EN_PORT, PWR_EN_PIN, GPIO_PIN_RESET);
    SIM_Delay(CMD_DELAY_SHORT);

    HAL_GPIO_WritePin(PWRKEY_PORT, PWRKEY_PIN, GPIO_PIN_SET);
    SIM_Delay(CMD_DELAY_MEDIUM);
    HAL_GPIO_WritePin(PWRKEY_PORT, PWRKEY_PIN, GPIO_PIN_RESET);

    while ((strstr(responseBuffer, "PB DONE") == NULL) && ((HAL_GetTick() - timeout) <= 30000))   SIM_Delay(CMD_DELAY_VERYSHORT);
    if ((HAL_GetTick() - timeout) >= 30000)
    {
        logPC("SIM - Module boot failed!");
        return false;
    }
    logPC("SIM - Module boot done");

    SIM_Delay(CMD_DELAY_SHORT);

    logPC("SIM - Initialize start...");
    if (SIM_sendATCommandResponse("AT\r\n", "OK", CMD_DELAY_MEDIUM) == false)
    {
        logPC("SIM - Initialize FAIL!");
        return false;
    }
    else
    {
        bool status = SIM_Init_Func();
        if (status == true)     logPC("SIM - Initialize OK!");
        else                    logPC("SIM - Initialize FAIL!");
        return status;
    }
}

bool SIM_Deinit(void)
{
    logPC("SIM - PWRDN");
    HAL_GPIO_WritePin(PWR_EN_PORT, PWR_EN_PIN, GPIO_PIN_SET);
    SIM_Delay(CMD_DELAY_VERYSHORT);
    return true;
}

void SIM_Reset(void)
{
    HAL_GPIO_WritePin(RESET_PORT, RESET_PIN, GPIO_PIN_SET);
    SIM_Delay(CMD_DELAY_MEDIUM);
    HAL_GPIO_WritePin(RESET_PORT, RESET_PIN, GPIO_PIN_SET);

    while (strstr(responseBuffer, "PB DONE") == NULL)   SIM_Delay(CMD_DELAY_VERYSHORT);
    logPC("SIM - Module boot done");
}

//----------------------------------------------------------------------------------

bool SIM_checkSIMCard(void)
{
    bool status = true;
    status &= SIM_sendATCommandResponse("AT+CPIN?\r\n", "READY", CMD_DELAY_VERYSHORT);
    status &= SIM_sendATCommandResponse("AT+CREG?\r\n", ",1", CMD_DELAY_VERYSHORT);
    return status;
}

void SIM_sendSMS(char* number, char* message)
{

}

void SIM_call(char* number)
{

}

bool SIM_startGPRS(void)
{
    bool status = true;

    status &= SIM_sendATCommandResponse("AT+CGATT=1\r\n", "OK", CMD_DELAY_VERYSHORT);
    SIM_Delay(CMD_DELAY_VERYSHORT);
    status &= SIM_sendATCommandResponse("AT+CGDCONT=1,\"IP\",\"m3-world\"\r\n", "OK", CMD_DELAY_VERYSHORT);
    status &= SIM_sendATCommandResponse("AT+CIPMODE=1\r\n", "OK", CMD_DELAY_VERYSHORT);
    status &= SIM_sendATCommandResponse("AT+CSOCKSETPN=1\r\n", "OK", CMD_DELAY_VERYSHORT);
    status &= SIM_sendATCommandResponse("AT+CIPMODE=0\r\n", "OK", CMD_DELAY_VERYSHORT);
    status &= SIM_sendATCommandResponse("AT+NETOPEN\r\n", "+NETOPEN: 0", CMD_DELAY_VERYSHORT);
    SIM_Delay(CMD_DELAY_VERYSHORT);
    status &= SIM_getIP();
    // status &= SIM_sendATCommandResponse("AT+NETCLOSE\r\n", "0", CMD_DELAY_VERYSHORT);

    status = true;
    return status;
}

bool SIM_getIP(void)
{
    uint32_t timeOut = HAL_GetTick();
    isWaiting4Response = true;

    HAL_UART_Transmit(UART_SIM, (unsigned char *)"AT+IPADDR\r\n", (uint16_t) 10, 100);

    while ((isWaiting4Response == true) && ((HAL_GetTick() - timeOut) <= CMD_DELAY_MEDIUM))     SIM_Delay(200);

    if (((HAL_GetTick() - timeOut) >= CMD_DELAY_MEDIUM))      return false;

    uint8_t dot = 0;
    SIM.IP.Octec1 = 0;
    SIM.IP.Octec2 = 0;
    SIM.IP.Octec3 = 0;
    SIM.IP.Octec4 = 0;

    dot = sscanf((const char*)responseBuffer, "+IPADDR: %i.%i.%i.%i", (int*)&SIM.IP.Octec1, (int*)&SIM.IP.Octec2, (int*)&SIM.IP.Octec3, (int*)&SIM.IP.Octec4);

    if (dot == 4)   return true;
    else            return false;
}

uint8_t SIM_checkSignalStrength(void)
{
    uint32_t timeOut = HAL_GetTick();
    isWaiting4Response = true;

    SIM_sendATCommand("AT+CSQ\r\n");

    while ((isWaiting4Response == true) && ((HAL_GetTick() - timeOut) <= CMD_DELAY_MEDIUM))     SIM_Delay(200);

    if (((HAL_GetTick() - timeOut) >= CMD_DELAY_MEDIUM))      return 0;

    sscanf((const char*)responseBuffer, "+CSQ: %i,", (int*)&SIM.signalQuality);

    return SIM.signalQuality;
}






