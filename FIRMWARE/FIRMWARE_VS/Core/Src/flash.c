/**
 * @file flash.c
 * @author hoanghuyhust (hoangnh191855@sis.hust.edu.vn)
 * @brief 
 * @version 0.1
 * @date 2023-12-01
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include "flash.h"

uint8_t rxbuff[256] = {0};
uint8_t idbuffer[5] = {0};

/*
#define FLASH_SS_Pin GPIO_PIN_12
#define FLASH_SS_GPIO_Port GPIOB
#define FLASH_CLK_Pin GPIO_PIN_13
#define FLASH_CLK_GPIO_Port GPIOB
#define FLASH_MISO_Pin GPIO_PIN_14
#define FLASH_MISO_GPIO_Port GPIOB
#define FLASH_MOSI_Pin GPIO_PIN_15
#define FLASH_MOSI_GPIO_Port GPIOB
*/
void CS_reset()
{
    HAL_GPIO_WritePin(FLASH_SS_GPIO_Port, FLASH_SS_Pin, GPIO_PIN_RESET);
}

void CS_set()
{
    HAL_GPIO_WritePin(FLASH_SS_GPIO_Port, FLASH_SS_Pin, GPIO_PIN_SET);
}

uint8_t spi_sendRecive(uint8_t txData)
{
    uint8_t rxData;
    HAL_SPI_TransmitReceive(&hspi2, (uint8_t *)&txData, (uint8_t *)&rxData, 1, 1000);
    return rxData;
}

void w25q32_init()
{
    CS_reset();
    spi_sendRecive(READID);
    idbuffer[0] = spi_sendRecive(0x00);
    idbuffer[1] = spi_sendRecive(0x00);
    idbuffer[2] = spi_sendRecive(0x00);
    CS_set();
    if (idbuffer[0] != 0xef || idbuffer[1] != 0x40 || idbuffer[2] != 0x16)
    {
        log_data("Flash init fail\n");
        while (1)
            ;
    }
}

void w25q32_sectorErase(uint32_t u32Address)
{
    CS_reset();
    // write enable
    spi_sendRecive(WRITE_ENABLE);
    CS_set();

    HAL_Delay(1);

    CS_reset();
    spi_sendRecive(ERASE_SECTOR);
    spi_sendRecive(u32Address >> 16);
    spi_sendRecive(u32Address >> 8);
    spi_sendRecive(u32Address);
    CS_set();

    HAL_Delay(400);
}
void w25q32_pageWrite(uint32_t u32Address, uint8_t *u8data)
{
    int len = strlen((const char*)u8data);
    CS_reset();
    // write enable
    spi_sendRecive(WRITE_ENABLE);
    CS_set();

    HAL_Delay(1);

    CS_reset();
    spi_sendRecive(WRITE_PAGE);
    spi_sendRecive(u32Address >> 16);
    spi_sendRecive(u32Address >> 8);
    spi_sendRecive(u32Address);
    for (int i = 0; i < len; i++)
    {
        spi_sendRecive(u8data[i]);
    }
    CS_set();
    HAL_Delay(3);
}
void w25q32_read(uint32_t u32Address, uint8_t *u8data, uint32_t len)
{
    CS_reset();
    spi_sendRecive(READ_DATA);
    spi_sendRecive(u32Address >> 16);
    spi_sendRecive(u32Address >> 8);
    spi_sendRecive(u32Address);
    for (uint32_t i = 0; i < len; i++)
    {
        u8data[i] = spi_sendRecive(0x00);
    }
    CS_set();
}