/*
 * platform.c
 *
 *  Created on: Oct 10, 2024
 *      Author: mfxjx
 */

#include "fm25_platform.h"
#include "fm25cl64.h"
#include "spi_drv.h"
#include "main.h"
#define SPI_BUFFER_MAX_SIZE 240
static uint8_t spiTxBuffer[SPI_BUFFER_MAX_SIZE];
static uint8_t spiRxBuffer[SPI_BUFFER_MAX_SIZE];
/*读FM25寄存器值*/
void ReadDataFromFM25(uint8_t *rData,uint16_t rSize)
{
       spiBeginTransaction();
       spi2Exchange(SPI2, rSize, spiTxBuffer, rData);
       spiEndTransaction();

}

/*写FM25寄存器值*/
void WriteDataToFM25(uint8_t *wData,uint16_t wSize)
{
		spiBeginTransaction();
	    spi2Exchange(SPI2, wSize, wData, spiRxBuffer);
	    spiEndTransaction();
}

/*片选操作*/
void ChipSelectForFM25(FM25CSType cs)
{
       if(cs==FM25CS_Enable)
       {
    	   LL_GPIO_ResetOutputPin(FM25CLXX_CS_GPIO_Port, FM25CLXX_CS_Pin);
       }
       else
       {
    	   LL_GPIO_SetOutputPin(FM25CLXX_CS_GPIO_Port, FM25CLXX_CS_Pin);
       }
}

