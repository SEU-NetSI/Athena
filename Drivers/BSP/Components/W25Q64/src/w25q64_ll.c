/*
 * w25q64_ll.c
 *
 *  Created on: May 20, 2024
 *      Author: mfxjx
 */

#include <stdio.h>
#include <stdint.h>
#include "w25q64_ll.h"
#include "spi_drv.h"

uint8_t LL_BSP_W25Qx_Init(void)
{
	LL_SPI_Enable(SPI1);
	BSP_W25Qx_Reset();
	return BSP_W25Qx_GetStatus();
}

static void	LL_BSP_W25Qx_Reset(void)
{
	W25Qx_Enable();
	spi_transmit(SPI1, 0x66);
	spi_transmit(SPI1, 0x99);
	W25Qx_Disable();
}

static uint8_t LL_BSP_W25Qx_GetStatus(void)
{
	uint8_t status;
	W25Qx_Enable();
	spi_transmit(SPI1, 0x05);
	spi_transmit_receive(SPI1, 0x00, &status);
	W25Qx_Disable();

	if((status & W25Q128FV_FSR_BUSY) != 0)
	{
		return W25Qx_BUSY;
	}
	else
	{
		return W25Qx_OK;
	}
}

void LL_BSP_W25Qx_Read_ID(uint8_t *ID)
{
	LL_SPI_Enable(SPI1);
	W25Qx_Enable();
	spi_transmit(SPI1, 0x90);
	spi_transmit(SPI1, 0x00);
	spi_transmit(SPI1, 0x00);
	spi_transmit(SPI1, 0x00);
	spi_transmit_receive(SPI1, 0x00, &ID[0]);
	spi_transmit_receive(SPI1, 0x00, &ID[1]);
	spi_transmit_receive(SPI1, 0x00, &ID[2]);
	spi_transmit_receive(SPI1, 0x00, &ID[3]);
	W25Qx_Disable();
}
