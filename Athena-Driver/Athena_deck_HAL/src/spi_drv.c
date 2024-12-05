/*
 * spi_drv.c
 *
 *  Created on: Apr 30, 2024
 *      Author: Yunfan,Hanjie
 */

#include <stdio.h>
#include <stdint.h>
#include "spi_drv.h"
#include "stm32h7xx_ll_dma.h"
#include "stm32h7xx_ll_spi.h"
#include "stm32h7xx_ll_bus.h"


bool spi2Exchange(SPI_TypeDef* SPIx, size_t length, const uint8_t* data_tx, uint8_t * data_rx)
{
	LL_DMA_ConfigAddresses(DMA1, LL_DMA_STREAM_1, (uint32_t)data_tx, LL_SPI_DMA_GetTxRegAddr(SPI2), LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
	LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_1, length);
	LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_1);
	LL_SPI_EnableDMAReq_TX(SPI2);

	LL_DMA_ConfigAddresses(DMA1, LL_DMA_STREAM_0, LL_SPI_DMA_GetRxRegAddr(SPI2), (uint32_t)data_rx, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
	LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_0, length);
	LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_0);
	LL_SPI_EnableDMAReq_RX(SPI2);
	LL_SPI_Enable(SPI2);
	LL_SPI_StartMasterTransfer(SPI2);
    // Wait for completion
    bool result = (xSemaphoreTake(txComplete, portMAX_DELAY) == pdTRUE)
             && (xSemaphoreTake(rxComplete, portMAX_DELAY) == pdTRUE);

    return result;
}

void spiBeginTransaction()
{
	xSemaphoreTake(spiMutex, portMAX_DELAY);

}

void spiEndTransaction()
{
	xSemaphoreGive(spiMutex);
}

