/*
 * dw3000deck_ll.c
 *
 *  Created on: Aug 4, 2024
 *      Author: twinhorse
 */

#include <string.h>
#include "dw3000deck_ll.h"

#define DW3000Deck_Enable()          LL_GPIO_ResetOutputPin(DW3000Deck_CS_GPIO_Port, DW3000Deck_CS_Pin)
#define DW3000Deck_Disable()         LL_GPIO_SetOutputPin(DW3000Deck_CS_GPIO_Port, DW3000Deck_CS_Pin)

#define SPI_BUFFER_MAX_SIZE 240
static uint8_t spiTxBuffer[SPI_BUFFER_MAX_SIZE];
static uint8_t spiRxBuffer[SPI_BUFFER_MAX_SIZE];

void spiDeckRead(const void* cmd,
			size_t cmdLength,
			void *data,
			size_t dataLength)
{
	spiDeckBeginTransaction();
	DW3000Deck_Enable();
	memcpy(spiTxBuffer, cmd, cmdLength);
	memset(spiTxBuffer + cmdLength, DUMMY_BYTE, dataLength);
	spiDeckExchange(cmdLength + dataLength, spiTxBuffer, spiRxBuffer);
	memcpy(data, spiRxBuffer + cmdLength, dataLength);
	DW3000Deck_Disable();
	spiDeckEndTransaction();
}

void BSP_DW3000_Read_ID(uint32_t *dw3000ID)
{

	/* dwt_readdevid(void)
	 * - dwt_read32bitoffsetreg(DEV_ID_ID, 0);
	 * dwt_read32bitoffsetreg(regFileID=0, regOffset = 0)
	 * - dwt_readfromdevice(regFileID=0,regOffset=0,4,buffer);
	 * void dwt_readfromdevice( regFileID=0, index=0,  length=4, *buffer)
	 * - dwt_xfer3000(regFileID=0, index=0, length=4, buffer, DW3000_SPI_RD_BIT);
	 * void dwt_xfer3000(regFileID=0, indx=0, length=4, *buffer, mode=0)
	 * - readfromspi(cnt, header, length, buffer)
	 * readfromspi( headerLength, *headerBuffer, readlength, *readBuffer)
	 * - spiRead(headerBuffer, headerLength, readBuffer, readlength);
	 */
	uint8_t cmd[4] = {0x40, DUMMY_BYTE, DUMMY_BYTE, DUMMY_BYTE};
	spiDeckRead(cmd, 4, dw3000ID, 4);

	return ;
}
