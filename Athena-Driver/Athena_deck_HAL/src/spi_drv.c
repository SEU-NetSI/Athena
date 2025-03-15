/*
 * spi_drv.c
 *
 *  Created on: Apr 30, 2024
 *      Author: Yunfan,Hanjie
 */

#include <stdio.h>
#include <stdint.h>
#include "spi_drv.h"
#include "main.h"
#include "stm32h7xx_ll_dma.h"
#include "stm32h7xx_ll_spi.h"
#include "stm32h7xx_ll_bus.h"
#include "stm32h7xx_ll_gpio.h"

#define SPI_TIMEOUT_MS         5      // 单次传输超时时间（ms）
#define ERROR_THRESHOLD        3      // 错误计数阈值
#define MAX_RETRIES            5      // 最大重试次数
#define ERROR_GPIO_PIN         LL_GPIO_PIN_9
#define ERROR_GPIO_PORT        GPIOE

bool spiDeckExchange(size_t length, const uint8_t* data_tx, uint8_t * data_rx)
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
    bool result = (xSemaphoreTake(spiDeckRxComplete, portMAX_DELAY) == pdTRUE)
             && (xSemaphoreTake(spiDeckTxComplete, portMAX_DELAY) == pdTRUE);

    return result;
}

// 静态变量用于错误计数（实际应用中建议使用原子操作）
static volatile uint8_t spiErrorCount = 0;

bool spi2Exchange(SPI_TypeDef* SPIx, size_t length,
                const uint8_t* data_tx, uint8_t* data_rx)
{
    uint8_t retries = 0;
    const TickType_t xTicksToWait = pdMS_TO_TICKS(SPI_TIMEOUT_MS);

    // 超过最大重试次数
    return false;
}

void spiBeginTransaction()
{
	return;
}

void spiEndTransaction()
{
	return;
}

//======

void spiDeckBeginTransaction()
{
	xSemaphoreTake(spiDeckMutex, portMAX_DELAY);

}

void spiDeckEndTransaction()
{
	xSemaphoreGive(spiDeckMutex);
}
