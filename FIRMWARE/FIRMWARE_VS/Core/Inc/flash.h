#ifndef _FLASH_H__
#define _FLASH_H__
#include "main.h"
#include "debug.h"

#define READID          0x9F
#define ERASE_SECTOR    0x20   
#define WRITE_ENABLE    0x06
#define WRITE_PAGE      0x02
#define READ_DATA       0x03

extern SPI_HandleTypeDef hspi2;

void w25q32_init(void);
uint8_t spi_sendRecive(uint8_t txData);
void w25q32_sectorErase(uint32_t u32Address);
void w25q32_pageWrite(uint32_t u32Address, uint8_t *u8data);
void w25q32_read(uint32_t u32Address, uint8_t *u8data, uint32_t len);

#endif