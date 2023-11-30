/**
 * @file sim4g_lte.c
 * @author hoanghuyhust (hoangnh191855@sis.hust.edu.vn)
 * @brief
 * @version 0.1
 * @date 2023-11-05
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "sim4g_lte.h"

char simRxBuf[200] = {0};
char simRxData;
simRxCplt_t simRxCplt = SIM_NOT_RX_CPLT;
char simRxResponse[200] = {0};
volatile int simRxIndex = 0;
volatile bool iswaiting4response = true;
simInfo_t sim7672 = {"m-wap", "mms", "mms", 0, 0};



void sim7672_reset(void)
{
    HAL_GPIO_WritePin(SIM_RESET_GPIO_Port, SIM_RESET_Pin, GPIO_PIN_RESET);
    HAL_Delay(50);
    HAL_GPIO_WritePin(SIM_RESET_GPIO_Port, SIM_RESET_Pin, GPIO_PIN_SET);
    HAL_Delay(2500);
    HAL_GPIO_WritePin(SIM_RESET_GPIO_Port, SIM_RESET_Pin, GPIO_PIN_RESET);
    log_data("SIM RESET\n");
}

void sim7672_pwrOff(void)
{
    HAL_GPIO_WritePin(SIM_CTRL_PWR_GPIO_Port, SIM_CTRL_PWR_Pin, GPIO_PIN_SET);
    HAL_Delay(100);
}

void sim7672_pwrOn(void)
{
    // EN power
    HAL_GPIO_WritePin(SIM_CTRL_PWR_GPIO_Port, SIM_CTRL_PWR_Pin, GPIO_PIN_RESET);
    HAL_Delay(100);
}

void sim7672_init(void)
{
    HAL_GPIO_WritePin(SIM_PWRKEY_GPIO_Port, SIM_PWRKEY_Pin, GPIO_PIN_SET);
    HAL_Delay(50);
    HAL_GPIO_WritePin(SIM_PWRKEY_GPIO_Port, SIM_PWRKEY_Pin, GPIO_PIN_RESET);
    HAL_Delay(50);
    HAL_GPIO_WritePin(SIM_PWRKEY_GPIO_Port, SIM_PWRKEY_Pin, GPIO_PIN_SET);
    HAL_Delay(50);
    HAL_GPIO_WritePin(SIM_PWRKEY_GPIO_Port, SIM_PWRKEY_Pin, GPIO_PIN_RESET);
    HAL_Delay(50);
    log_data("SIM PWERKEY\n");

    if (sim7672_send_command("ATE0\r\n", "OK\r\n",TIME_VERY_SHORT) != SIM_TRUE)
    {
        sim7672_errorHandle();
    }
    HAL_Delay(100);
    if (sim7672_send_command("AT\r\n", "OK\r\n",TIME_VERY_SHORT) != SIM_TRUE)
    {
        sim7672_errorHandle();
    }
    HAL_Delay(100);
    if (sim7672_send_command("AT+CFUN?\r\n", "1\r\n",TIME_VERY_SHORT) != SIM_TRUE)
    {
        sim7672_errorHandle();
    }
    HAL_Delay(100);
    
    sim7672_checkSIMCard();

}

void sim7672_sendcmd_notresp(char *cmd)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)cmd, strlen(cmd), 100);
}

simState_t sim7672_send_command(char *simCommand, char *trueResponse, int waitms)
{
    memset(simRxResponse, '\0', strlen(simRxResponse));
    memset(simRxBuf, '\0', strlen(simRxBuf));
    simRxIndex = 0;

    uint32_t timeout = HAL_GetTick();

    HAL_UART_Receive_IT(&huart1, (uint8_t *)&simRxData, 1);
    HAL_UART_Transmit(&huart1, (uint8_t *)simCommand, strlen(simCommand), 100);

    log_data(simCommand);

    while (HAL_GetTick()-timeout<=waitms)
    {
        HAL_Delay(50);
    }
    memcpy(simRxResponse, simRxBuf, strlen(simRxBuf));
    log_data(simRxResponse);
    if ((strstr(simRxResponse, trueResponse) == NULL))
    {
        memset(simRxResponse, '\0', strlen(simRxResponse));
        // simRxCplt = SIM_NOT_RX_CPLT;
        return SIM_FALSE;
    }

    // simRxCplt = SIM_NOT_RX_CPLT;
    iswaiting4response = true;
    return SIM_TRUE;
}

// Receive response from SIM and delete Buffer
// message response format: \r\n[Respone..]\r\\n
void sim7672_callback(void)
{
    simRxBuf[simRxIndex++] = simRxData;

    if ((strstr(simRxBuf, "\r\n") != NULL) && (simRxIndex == 2))
    {
        memset(simRxBuf, '\0', strlen(simRxBuf));
        simRxIndex = 0;
    }
    // if ((strstr(simRxBuf, "\r\n") != NULL))
    // {
    //     // log_data(simRxBuf);
    //     memcpy(simRxResponse, simRxBuf, strlen(simRxBuf));
    //     memset(simRxBuf, '\0', strlen(simRxBuf));
    //     simRxIndex = 0;
    //     iswaiting4response = false;
    // }
    HAL_UART_Receive_IT(&huart1, (uint8_t *)&simRxData, 1);
}

/*
2 Automatic
13 GSM Only
14 WCDMA Only
38 LTE Only
*/
void sim7672_start_LTE(void)
{
    // // check connection  "AT+CGATT=1\r\n"
    // if (sim7672_send_command("AT+CGATT=1\r\n", "OK\r\n") != SIM_TRUE)
    // {
    //     sim7672_errorHandle();
    // }
    // HAL_Delay(100);
    /*
    setting APN:
    mobi: APN: m-wap
    vina: APN: m3-world
    vietnammobile: APN: internet
    viettel: APN: v-internet
    */
    if (sim7672_send_command("AT+CGDCONT=1,\"IP\",\"v-internet\"\r\n", "OK\r\n",TIME_VERY_SHORT) != SIM_TRUE)
    {
        sim7672_errorHandle();
    }
    HAL_Delay(100);
    // // select network LTE
    // if (sim7672_send_command("AT+CNMP=38\r\n", "OK\r\n") != SIM_TRUE)
    // {
    //     sim7672_errorHandle();
    // }
    // HAL_Delay(100);
    // start socket: 0 success
    // if(sim7672_send_command("AT+NETCLOSE\r\n","OK\r\n")!=SIM_TRUE)
    // {
    //     sim7672_errorHandle();
    // }
    // HAL_Delay(100);
    // select band any
    // if(sim7672_send_command("AT+CNBP=0x000700000FFF0380,0x000007FF3FDF3FFF\r\n", "OK\r\n")!=SIM_TRUE)
    // {
    //     sim7672_errorHandle();
    // }
    // HAL_Delay(100);
    // normal message
    // if(sim7672_send_command("AT+CIPMODE=0\r\n", "OK\r\n")!=SIM_TRUE)
    // {
    //     sim7672_errorHandle();
    // }
    // HAL_Delay(100);
    // start socket: 0 success
    // if(sim7672_send_command("AT+NETOPEN\r\n","OK\r\n")!=SIM_TRUE)
    // {
    //     sim7672_errorHandle();
    // }
    // HAL_Delay(100);
}

uint8_t sim7672_check_signalStrength(void)
{
    if (sim7672_send_command("AT+CSQ\r\n", "\r\n",TIME_VERY_SHORT) != SIM_TRUE)
    {
        sim7672_errorHandle();
    }
    sscanf((const char *)simRxResponse, "+CSQ: %i,", (int *)&sim7672.signalQuality);
    return sim7672.signalQuality;
}

void sim7672_checkSIMCard()
{
    if (sim7672_send_command("AT+CPIN?\r\n", "READY\r\n",TIME_VERY_SHORT) != SIM_TRUE)
    {
        sim7672_errorHandle();
    }
}

void sim7672_errorHandle(void)
{
    log_data("sim error\n");
    // __disable_irq();
    while (1)
    {
    }
}
