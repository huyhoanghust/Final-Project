#include "sim4g_lte.h"

char simRxBuf[100] = {0};
char simRxData;
simRxCplt_t simRxCplt = SIM_NOT_RX_CPLT;
char simRxResponse[100] = {0};
int simRxIndex = 0;

void sim7672_pwrkey(void)
{
    do
    {
        HAL_GPIO_WritePin(SIM_PWRKEY_GPIO_Port, SIM_PWRKEY_Pin, GPIO_PIN_SET);
        HAL_Delay(100);
    } while (HAL_GPIO_ReadPin(SIM_PWRKEY_GPIO_Port, SIM_PWRKEY_Pin) == GPIO_PIN_RESET);
    log_data("PWERKEY OKE\n");
}

void sim7672_init(void)
{
    if (sim7672_send_command("ATE0\r\n", "OK\r\n", SIM_NOT_CHECK) != SIM_OKE)
    {
        sim7672_errorHandle();
    }
    if (sim7672_send_command("AT\r\n", "OK\r\n", SIM_CHECK) != SIM_OKE)
    {
        sim7672_errorHandle();
    }
}

simState_t sim7672_send_command(char *simCommand, char *trueResponse, int checkOrNot)
{
    HAL_UART_Receive_IT(&huart1, (uint8_t *)&simRxData, 1);
    HAL_UART_Transmit(&huart1, (uint8_t *)simCommand, strlen(simCommand), 100);
    log_data(simCommand);

    HAL_TIM_Base_Start_IT(&htim3);

    // Wait until complete receiving respone from SIM
    while (!(simRxCplt == SIM_RX_CPLT))
        ;
    if ((checkOrNot == SIM_CHECK) && (strstr(simRxResponse, trueResponse) == NULL))
    {
        memset(simRxResponse, '\0', strlen(simRxResponse));
        simRxCplt = SIM_NOT_RX_CPLT;
        return SIM_NOT_OKE;
    }
    memset(simRxResponse, '\0', strlen(simRxResponse));
    simRxCplt = SIM_NOT_RX_CPLT;
    return SIM_OKE;
}

// Receive response from SIM and delete Buffer
void sim7672_callback(void)
{
    htim3.Instance->CNT = 0;
    simRxBuf[simRxIndex++] = simRxData;
    HAL_UART_Receive_IT(&huart1, (uint8_t *)&simRxData, 1);
    if ((strstr(simRxBuf, "\r\n") != NULL) && (simRxIndex == 2))
    {
        memset(simRxBuf, '\0', strlen(simRxBuf));
        simRxIndex = 0;
    }
}
// wait timeout, once receiving complete wait 0.2s to check remain response
void sim7672_timerCallback(void)
{
    HAL_TIM_Base_Stop_IT(&htim3);
    htim3.Instance->CNT = 0;
    if ((strstr(simRxBuf, "\r\n") != NULL) | (strstr(simRxBuf, ">") != NULL))
    {
        memcpy(simRxResponse, simRxBuf, strlen(simRxBuf));
        memset(simRxBuf, '\0', strlen(simRxBuf));
        simRxIndex = 0;
    }
    simRxCplt = SIM_RX_CPLT;
}

void sim7672_errorHandle(void)
{
    log_data("sim error");
    __disable_irq();
    while (1)
    {
    }
}
