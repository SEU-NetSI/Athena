/*
 * spi_deck.c
 *
 *  Created on: Aug 4, 2024
 *      Author: Feng Shan
 */

#include <stdio.h>
#include <stdint.h>
#include "spi_deck.h"
#include "main.h"
#include "dw3000_cbll.h"


/*
  1. SPI2
  2. DMA1
    a. channel 4: read from SPI2_RX
    b. channel 5: write to SPI2_TX
  */
bool spiDeckExchange(size_t length, const uint8_t * data_tx, uint8_t * data_rx)
{
    SPI_TypeDef* SPIx = SPI2;
    DMA_TypeDef* DMAx = DMA1;

	// DMA already configured, just need to set memory addresses
    LL_DMA_ConfigAddresses(DMAx, LL_DMA_CHANNEL_5, (uint32_t)data_tx, LL_SPI_DMA_GetRegAddr(SPIx), LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
    LL_DMA_SetDataLength(DMAx, LL_DMA_CHANNEL_5, length);

    LL_DMA_ConfigAddresses(DMAx, LL_DMA_CHANNEL_4, LL_SPI_DMA_GetRegAddr(SPIx), (uint32_t)data_rx, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
    LL_DMA_SetDataLength(DMAx, LL_DMA_CHANNEL_4, length);

    // Enable DMA Streams
    LL_DMA_EnableChannel(DMAx, LL_DMA_CHANNEL_5);
    LL_DMA_EnableChannel(DMAx, LL_DMA_CHANNEL_4);

    // Enable SPI DMA requests
    LL_SPI_EnableDMAReq_TX(SPIx);
    LL_SPI_EnableDMAReq_RX(SPIx);

    // Enable peripheral
    LL_SPI_Enable(SPIx);

    // Wait for completion
    bool result = (xSemaphoreTake(spiDeckTxComplete, portMAX_DELAY) == pdTRUE)
             && (xSemaphoreTake(spiDeckRxComplete, portMAX_DELAY) == pdTRUE);

    // Disable peripheral
    LL_SPI_Disable(SPIx);
    return result;
}

void spiDeckBeginTransaction()
{
	xSemaphoreTake(spiDeckMutex, portMAX_DELAY);

}

void spiDeckEndTransaction()
{
	xSemaphoreGive(spiDeckMutex);
}
