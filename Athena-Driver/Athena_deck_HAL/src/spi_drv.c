/*
 * spi_drv.c
 *
 *  Created on: Apr 30, 2024
 *      Author: Yunfan,Hanjie
 */

#include <stdio.h>
#include <stdint.h>
#include "spi_drv.h"
#include "stm32l4xx_ll_dma.h"
#include "stm32l4xx_ll_spi.h"
#include "stm32l4xx_ll_bus.h"


bool spiExchange(SPI_TypeDef* SPIx, size_t length, const uint8_t * data_tx, uint8_t * data_rx)
{
	// DMA already configured, just need to set memory addresses
    LL_DMA_ConfigAddresses(DMA2, LL_DMA_CHANNEL_2, (uint32_t)data_tx, LL_SPI_DMA_GetRegAddr(SPIx), LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
    LL_DMA_SetDataLength(DMA2, LL_DMA_CHANNEL_2, length);

    LL_DMA_ConfigAddresses(DMA2, LL_DMA_CHANNEL_1, LL_SPI_DMA_GetRegAddr(SPIx), (uint32_t)data_rx, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
    LL_DMA_SetDataLength(DMA2, LL_DMA_CHANNEL_1, length);

    // Enable DMA Streams
    LL_DMA_EnableChannel(DMA2, LL_DMA_CHANNEL_2);
    LL_DMA_EnableChannel(DMA2, LL_DMA_CHANNEL_1);

    // Enable SPI DMA requests
    LL_SPI_EnableDMAReq_TX(SPIx);
    LL_SPI_EnableDMAReq_RX(SPIx);

    // Enable peripheral
    LL_SPI_Enable(SPIx);


    // Wait for completion
    bool result = (xSemaphoreTake(txComplete, portMAX_DELAY) == pdTRUE)
             && (xSemaphoreTake(rxComplete, portMAX_DELAY) == pdTRUE);

    // Disable peripheral
    LL_SPI_Disable(SPIx);
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

