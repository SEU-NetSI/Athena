/*
 * w25q64_ll.c
 *
 *  Created on: May 20, 2024
 *      Author: mfxjx
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "w25q64_ll.h"
#include "spi_drv.h"

#define SPI_BUFFER_MAX_SIZE 240
static uint8_t spiTxBuffer[SPI_BUFFER_MAX_SIZE];
static uint8_t spiRxBuffer[SPI_BUFFER_MAX_SIZE];

void spiRead(const void* cmd,
			size_t cmdLength,
			void *data,
			size_t dataLength,
			SPI_TypeDef* SPIx)
{
	spiBeginTransaction();
	W25Qx_Enable();
	memcpy(spiTxBuffer, cmd, cmdLength);
	memset(spiTxBuffer + cmdLength, DUMMY_BYTE, dataLength);
	spiExchange(SPIx, cmdLength + dataLength, spiTxBuffer, spiRxBuffer);
	memcpy(data, spiRxBuffer + cmdLength, dataLength);
	W25Qx_Disable();
	spiEndTransaction();
}


uint8_t BSP_W25Qx_Init(void)
{
	BSP_W25Qx_Reset();
	return BSP_W25Qx_GetStatus();
}


static void	BSP_W25Qx_Reset(void)
{
	uint8_t cmd[2]= {RESET_ENABLE_CMD,RESET_MEMORY_CMD};
	uint8_t dummy[]= {DUMMY_BYTE};
	spiRead(cmd, 2, dummy, 1, SPI3);
	vTaskDelay(1);
}


static uint8_t BSP_W25Qx_GetStatus(void)
{
	uint8_t cmd[] = {READ_STATUS_REG1_CMD};
	uint8_t status= 0xFF;
	spiRead(cmd, 1, &status, 1, SPI3);

	if((status & W25Q128FV_FSR_BUSY) != 0)
	{
		return W25Qx_BUSY;
	}
	else
	{
		return W25Qx_OK;
	}
}

void BSP_W25Qx_Read_ID(uint8_t *ID)
{
	uint8_t cmd[4] = {READ_ID_CMD, DUMMY_BYTE, DUMMY_BYTE, 0x00};
	spiRead(cmd, 4, ID, 2, SPI3);
}

