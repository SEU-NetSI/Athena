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

void BSP_W25Qx_Read_ID(uint8_t *ID)
{
	uint8_t cmd[4] = {READ_ID_CMD,0x00,0x00,0x00};
	uint8_t dummy[4]={0};
	uint8_t dummy_rx[2]={0};

	W25Qx_Enable();
	/* Send the read ID command */
	spiBeginTransaction();

	if (spiExchange(SPI3, sizeof(cmd), cmd, dummy))
	{

	}
	else
	{
		while(1);
	}


	if (spiExchange(SPI3, sizeof(dummy_rx), dummy_rx, ID))
	{

	}
	else
	{
		while(1);
	}
	spiEndTransaction();
	W25Qx_Disable();

	return;

}
