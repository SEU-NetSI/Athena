/*
 * fm25_platform.c
 *
 *  Created on: Nov 23, 2024
 *      Author: Hanjie Yu
 */

#include "fm25_platform.h"
#include "fm25cl64.h"
#include "spi_drv.h"
#include "main.h"
#define SPI_BUFFER_MAX_SIZE 520
static uint8_t spiTxBuffer[SPI_BUFFER_MAX_SIZE];
static uint8_t spiRxBuffer[SPI_BUFFER_MAX_SIZE];
/*读FM25寄存器值*/

void ReadDataFromFM25(uint8_t *rData, uint16_t rSize) {
	spi1BeginTransaction();
	spi1Exchange(SPI1, rSize, spiTxBuffer, rData);
	spi1EndTransaction();
}

/*写FM25寄存器值*/
void WriteDataToFM25(uint8_t *wData, uint16_t wSize) {
	spi1BeginTransaction();
	spi1Exchange(SPI1, wSize, wData, spiRxBuffer);
	spi1EndTransaction();
}

/*片选操作*/
void ChipSelectForFM25(FM25CSType cs) {
	if (cs == FM25CS_Enable) {
		LL_GPIO_ResetOutputPin(FM25CLXX_CS_GPIO_Port, FM25CLXX_CS_Pin);
	} else {
		LL_GPIO_SetOutputPin(FM25CLXX_CS_GPIO_Port, FM25CLXX_CS_Pin);
	}
}

