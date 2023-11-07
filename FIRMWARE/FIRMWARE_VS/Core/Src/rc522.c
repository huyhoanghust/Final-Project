/**
 * @file rc522.c
 * @author hoanghuyhust (hoangnh191855@sis.hust.edu.vn)
 * @brief 
 * @version 0.1
 * @date 2022-09-25
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "rc522.h"

/*
 * Function Name: rc522_writeRegister
 * Function Description: To a certain MFRC522 register to write a byte of data
 * Input Parameter: addr - register address; val - the value to be written
 * Return value: None
 */
void rc522_writeRegister(uint8_t addr, uint8_t val)
{
	uint8_t arr[2] = {0};
    addr = (addr << 1) & 0x7E; // Address format: 0XXXXXX0
	*arr = addr;
	*(arr + 1) = val;
	// CS Low
    HAL_GPIO_WritePin(RC522_CS_GPIO_Port, RC522_CS_Pin, RESET);
	// send 2 bytes
	//  - top 8 bits are the address. Per the spec, we shift the address left
	//   1 bit, clear the LSb, and clear the MSb to indicate a write
	// - bottom 8 bits are the data bits being sent for that address,
	HAL_SPI_Transmit(&hspi1, arr, 2, 100);
	// cs high
    HAL_GPIO_WritePin(RC522_CS_GPIO_Port, RC522_CS_Pin, SET);
}

/*
 * Function Name: rc522_readRegister
 * Description: From a certain MFRC522 read a byte of data register
 * Input Parameters: addr - register address
 * Returns: a byte of data read from the register
 */
uint8_t rc522_readRegister(uint8_t addr)
{
    uint8_t val = 0, temp = 0x00;
    addr = ((addr << 1) & 0x7E) | 0x80; // Address format: 1XXXXXX0
    HAL_GPIO_WritePin(RC522_CS_GPIO_Port, RC522_CS_Pin, RESET);
    HAL_SPI_TransmitReceive(&hspi1, &addr, &val, 1, 100);
	HAL_SPI_TransmitReceive(&hspi1, &temp, &val, 1, 100);
    HAL_GPIO_WritePin(RC522_CS_GPIO_Port, RC522_CS_Pin, SET);
    return val;
}

/*
 * Function Name: rc522_init
 * Description: Initialize RC522
 * Input: None
 * Return value: None
*/
void rc522_init(void)
{
    rc522_reset();
    rc522_writeRegister(MFRC522_REG_T_MODE, 0x80);
    rc522_writeRegister(MFRC522_REG_T_PRESCALER, 0xA9);
    rc522_writeRegister(MFRC522_REG_T_RELOAD_L, 0xE8);
    rc522_writeRegister(MFRC522_REG_T_RELOAD_H, 0x03);
    rc522_writeRegister(MFRC522_REG_RF_CFG, 0x70);
    rc522_writeRegister(MFRC522_REG_TX_AUTO, 0x40);
    rc522_writeRegister(MFRC522_REG_MODE, 0x3D);
    rc522_controlAntenna(ANTENNA_ON);
}

/*
 * Function Name: rc522_controlAntenna
 * Description: Open or close antennas, each time you start or shut down the natural barrier between the transmitter should be at least 1ms interval
 * Input: ANTENNA_ON or OFF
 * Return value: None
 */
void rc522_controlAntenna(uint8_t mode)
{
	uint8_t temp = 0;
    if(mode == ANTENNA_OFF)
    {
        rc522_clearBitMask(MFRC522_REG_TX_CONTROL, 0x03);
    }
    else if(mode == ANTENNA_ON)
    {
		temp = rc522_readRegister(MFRC522_REG_TX_CONTROL);
		if(!(temp & 0x03))
		{
			rc522_setBitMask(MFRC522_REG_TX_CONTROL, 0x03);
		}
        
    }
}

/*
 * Function Name: rc522_reset
 * Description: Reset RC522
 * Input: None
 * Return value: None
 */
void rc522_reset(void)
{
    rc522_writeRegister(MFRC522_REG_COMMAND, PCD_RESETPHASE);
}


RC522_Status_t rc522_check(uint8_t *id)
{
    RC522_Status_t status;
    status = rc522_request(PICC_REQIDL, id);
    if(status == RC522_OK)
    {
        status = rc522_anticoll(id);
    }
    rc522_halt();
    return status;
}

/*
 * Function Name: rc522_request
 * Description: Find cards, read the card type number
 * Input parameters: reqMode - find cards way
 *   TagType - Return Card Type
 *    0x4400 = Mifare_UltraLight
 *    0x0400 = Mifare_One(S50)
 *    0x0200 = Mifare_One(S70)
 *    0x0800 = Mifare_Pro(X)
 *    0x4403 = Mifare_DESFire
 * Return value: the successful return MI_OK
 */
RC522_Status_t rc522_request(uint8_t reqMode, uint8_t *tagType)
{
    RC522_Status_t status;
    uint16_t backbits;
    rc522_writeRegister(MFRC522_REG_BIT_FRAMING, 0x07);
    tagType[0] = reqMode;
    status = rc522_toCard(PCD_TRANSCEIVE, tagType, 1, tagType, &backbits);
    if((status != RC522_OK) || (backbits != 0x10)) 
    {    
		status = RC522_ERR;
	}
	return status;
}

/*
 * Function Name: rc522_toCard
 * Description: RC522 and ISO14443 card communication
 * Input Parameters: command - MF522 command word,
 *			 sendData--RC522 sent to the card by the data
 *			 sendLen--Length of data sent
 *			 backData--Received the card returns data,
 *			 backLen--Return data bit length
 * Return value: the successful return MI_OK
 */
RC522_Status_t rc522_toCard(uint8_t command, uint8_t *sendData, uint8_t sendLen, uint8_t *backData, uint16_t *backLen)
{
    uint8_t status = RC522_ERR;
	uint8_t irqEn = 0x00;
	uint8_t waitIRq = 0x00;
	uint8_t lastBits;
	uint8_t n;
	uint16_t i;

	switch(command)
    {
		case PCD_AUTHENT:
        {
			irqEn = 0x12;
			waitIRq = 0x10;
			break;
		}
		case PCD_TRANSCEIVE:
        {
			irqEn = 0x77;
			waitIRq = 0x30;
			break;
		}
		default:
		    break;
	}
	rc522_writeRegister(MFRC522_REG_COMM_IE_N, irqEn | 0x80);
	rc522_clearBitMask(MFRC522_REG_COMM_IRQ, 0x80);
	rc522_setBitMask(MFRC522_REG_FIFO_LEVEL, 0x80);
	rc522_writeRegister(MFRC522_REG_COMMAND, PCD_IDLE);
	// Writing data to the FIFO
	for(i = 0; i < sendLen; i++) 
    {
        rc522_writeRegister(MFRC522_REG_FIFO_DATA, sendData[i]);
    }
	// Execute the command
	rc522_writeRegister(MFRC522_REG_COMMAND, command);
	if(command == PCD_TRANSCEIVE) 
    {
        rc522_setBitMask(MFRC522_REG_BIT_FRAMING, 0x80);		// StartSend=1, transmission of data starts 
    }
    // Waiting to receive data to complete
	i = 2000;	// i according to the clock frequency adjustment, the operator M1 card maximum waiting time 25ms
	do
    {
		// CommIrqReg[7..0]
		// Set1 TxIRq RxIRq IdleIRq HiAlerIRq LoAlertIRq ErrIRq TimerIRq
		n = rc522_readRegister(MFRC522_REG_COMM_IRQ);
		i--;
	} while ((i != 0) && !(n & 0x01) && !(n & waitIRq));
	rc522_clearBitMask(MFRC522_REG_BIT_FRAMING, 0x80);      // StartSend=0
	if(i != 0)
    {
		if(!(rc522_readRegister(MFRC522_REG_ERROR) & 0x1B)) 
        {
			status = RC522_OK;
			if(n & irqEn & 0x01)
            {
                status = RC522_NOTAGERR;
            }
            if(command == PCD_TRANSCEIVE) 
            {
				n = rc522_readRegister(MFRC522_REG_FIFO_LEVEL);
				lastBits = rc522_readRegister(MFRC522_REG_CONTROL) & 0x07;
				if(lastBits)
                { 
                    *backLen = (n - 1) * 8 + lastBits; 
                }
                else 
                {
                    *backLen = n * 8;
                }
                if(n == 0)
                {
                    n = 1;
                }
                if(n > MFRC522_MAX_LEN)
                {
                    n = MFRC522_MAX_LEN;
                }
                for(i = 0; i < n; i++)
                {
                    backData[i] = rc522_readRegister(MFRC522_REG_FIFO_DATA);		// Reading the received data in FIFO
                }
            }
		} 
        else 
        {
            status = RC522_ERR;
        }
    }
	return status;
}

/*
 * Function Name: rc522_setBitMask
 * Description: Set RC522 register bit
 * Input parameters: reg - register address; mask - set bit value
 * Return value: None
 */
void rc522_setBitMask(uint8_t reg, uint8_t mask) 
{
    uint8_t temp = rc522_readRegister(reg);
	rc522_writeRegister(reg, temp | mask);
}

/*
 * Function Name: rc522_clearBitMask
 * Description: clear RC522 register bit
 * Input parameters: reg - register address; mask - clear bit value
 * Return value: None
*/
void rc522_clearBitMask(uint8_t reg, uint8_t mask)
{
    uint8_t temp = rc522_readRegister(reg);
	rc522_writeRegister(reg, temp & (~mask));
}

/*
 * Function Name: rc522_anticoll
 * Description: Anti-collision detection, reading selected card serial number card
 * Input parameters: serNum - returns 4 bytes card serial number, the first 5 bytes for the checksum byte
 * Return value: the successful return MI_OK
 */
uint8_t rc522_anticoll(uint8_t *serNum)
{
	uint8_t status;
	uint8_t i;
	uint8_t serNumCheck = 0;
	uint16_t unLen;

	rc522_writeRegister(MFRC522_REG_BIT_FRAMING, 0x00);												// TxLastBists = BitFramingReg[2..0]
	serNum[0] = PICC_ANTICOLL;
	serNum[1] = 0x20;
	status = rc522_toCard(PCD_TRANSCEIVE, serNum, 2, serNum, &unLen);
	if(status == RC522_OK) 
    {
		// Check card serial number
		for(i = 0; i < 4; i++)
        {
            serNumCheck ^= serNum[i];
        } 
		if(serNumCheck != serNum[i])
        {
            status = RC522_ERR;
        } 
	}
	return status;
}

/*
 * Function Name: MFRC522_Halt
 * Description: Command card into hibernation
 * Input: None
 * Return value: None
 */
void rc522_halt(void)
{
    uint16_t unLen;
	uint8_t buff[4]; 

	buff[0] = PICC_HALT;
	buff[1] = 0;
	rc522_calculateCRC(buff, 2, &buff[2]);

	rc522_toCard(PCD_TRANSCEIVE, buff, 4, buff, &unLen);
}

/*
 * Function Name: rc522_calculateCRC
 * Description: CRC calculation with MF522
 * Input parameters: pIndata - To calculate the CRC data, len - the data length, pOutData - CRC calculation results
 * Return value: None
 */
void rc522_calculateCRC(uint8_t *pInData, uint8_t len, uint8_t *pOutData)
{
    uint8_t i, n;
	rc522_clearBitMask(MFRC522_REG_DIV_IRQ, 0x04);													// CRCIrq = 0
	rc522_setBitMask(MFRC522_REG_FIFO_LEVEL, 0x80);													// Clear the FIFO pointer
	// Writing data to the FIFO	
	for(i = 0; i < len; i++)
    {
        rc522_writeRegister(MFRC522_REG_FIFO_DATA, *(pInData + i));
    }
    rc522_writeRegister(MFRC522_REG_COMMAND, PCD_CALCCRC);

	// Wait CRC calculation is complete
	i = 0xFF;
	do 
    {
		n = rc522_readRegister(MFRC522_REG_DIV_IRQ);
		i--;
	} while((i != 0) && !(n & 0x04));																						// CRCIrq = 1

	// Read CRC calculation result
	pOutData[0] = rc522_readRegister(MFRC522_REG_CRC_RESULT_L);
	pOutData[1] = rc522_readRegister(MFRC522_REG_CRC_RESULT_M);
}

/*
 * Function Name: rc522_selectTag
 * Description: election card, read the card memory capacity
 * Input parameters: serNum - Incoming card serial number
 * Return value: the successful return of card capacity
 */
uint8_t rc522_selectTag(uint8_t *serNum)
{
    uint8_t i;
	RC522_Status_t status;
	uint8_t size;
	uint16_t recvBits;
	uint8_t buffer[9]; 

	buffer[0] = PICC_SELECTTAG;
	buffer[1] = 0x70;
	for (i = 0; i < 5; i++) 
    {
		buffer[i + 2] = *(serNum + i);
	}
	rc522_calculateCRC(buffer, 7, &buffer[7]);		
	status = rc522_toCard(PCD_TRANSCEIVE, buffer, 9, buffer, &recvBits);

	if((status == RC522_OK) && (recvBits == 0x18)) 
    {   
		size = buffer[0]; 
	} 
    else 
    {   
		size = 0;    
	}
	return size;
}

/*
 * Function Name: rc522_auth
 * Description: Verify card password
 * Input parameters: authMode - Password Authentication Mode
                 0x60 = A key authentication
                 0x61 = Authentication Key B
             BlockAddr--Block address
             Sectorkey--Sector password
             serNum--Card serial number, 4-byte
 * Return value: the successful return MI_OK
 */
RC522_Status_t rc522_auth(uint8_t authMode, uint8_t blockAddr, uint8_t* sectorkey, uint8_t* serNum)
{
    RC522_Status_t status;
	uint16_t recvBits;
	uint8_t i;
	uint8_t buff[12]; 
	//Verify the command block address + sector + password + card serial number
	buff[0] = authMode;
	buff[1] = blockAddr;
	for(i = 0; i < 6; i++) 
    {    
		buff[i + 2] = *(sectorkey + i);   
	}
	for(i = 0; i < 4; i++) 
    {    
		buff[i + 8] = *(serNum + i);   
	}
	status = rc522_toCard(PCD_AUTHENT, buff, 12, buff, &recvBits);
	if ((status != RC522_OK) ||(!(rc522_readRegister(MFRC522_REG_STATUS2) & 0x08))) 
    {   
		status = RC522_ERR;   
	}
	return status;
}

/*
 * Function Name: rc522_read
 * Description: Read block data
 * Input parameters: blockAddr - block address; recvData - read block data
 * Return value: the successful return MI_OK
 */
RC522_Status_t rc522_read(uint8_t blockAddr, uint8_t* recvData)
{
	RC522_Status_t status;
	uint16_t unLen;

	recvData[0] = PICC_READ;
	recvData[1] = blockAddr;
	rc522_calculateCRC(recvData,2, &recvData[2]);
	status = rc522_toCard(PCD_TRANSCEIVE, recvData, 4, recvData, &unLen);
	if((status != RC522_OK) || (unLen != 0x90)) 
    {
		status = RC522_ERR;
	}
	return status;
}

/*
 * Function Name: rc522_write
 * Description: Write block data
 * Input parameters: blockAddr - block address; writeData - to 16-byte data block write
 * Return value: the successful return MI_OK
 */
RC522_Status_t rc522_write(uint8_t blockAddr, uint8_t* writeData) 
{
	RC522_Status_t status;
	uint16_t recvBits;
	uint8_t i;
	uint8_t buff[18]; 

	buff[0] = PICC_WRITE;
	buff[1] = blockAddr;
	rc522_calculateCRC(buff, 2, &buff[2]);
	status = rc522_toCard(PCD_TRANSCEIVE, buff, 4, buff, &recvBits);

	if((status != RC522_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A)) 
    {   
		status = RC522_ERR;   
	}

	if(status == RC522_OK) 
    {
		//Data to the FIFO write 16Byte
		for(i = 0; i < 16; i++) 
        {    
			buff[i] = *(writeData + i);   
		}
		rc522_calculateCRC(buff, 16, &buff[16]);
		status = rc522_toCard(PCD_TRANSCEIVE, buff, 18, buff, &recvBits);

		if((status != RC522_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A)) 
        {   
			status = RC522_ERR;   
		}
	}
	return status;
}
