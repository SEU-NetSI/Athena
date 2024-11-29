/*
 * spi_drv.h
 *
 *  Created on: Apr 30, 2024
 *      Author: Yunfan, Hanjie
 */

#ifndef ATHENA_DECK_HAL_INC_SPI_DRV_H_
#define ATHENA_DECK_HAL_INC_SPI_DRV_H_

#include <stdbool.h>
#include "stm32l4xx.h"
#include "stm32l4xx_ll_spi.h"
#include "stm32l4xx_ll_dma.h"
#include "FreeRTOS.h"
#include "semphr.h"

typedef struct {
	SPI_TypeDef *SPIx;            // SPI外设
	DMA_TypeDef *DMAx;            // DMA外设
	uint32_t channel_tx;          // TX DMA通道
	uint32_t channel_rx;          // RX DMA通道
	size_t length;                // 数据长度
	const uint8_t *data_tx;       // 发送数据缓冲区
	uint8_t *data_rx;             // 接收数据缓冲区
	SemaphoreHandle_t spiMutex;	  //SPI总线占用信号量
	SemaphoreHandle_t txComplete; // TX 完成信号量
	SemaphoreHandle_t rxComplete; // RX 完成信号量
} SPI_Config_t;

extern SemaphoreHandle_t txComplete;
extern SemaphoreHandle_t rxComplete;
extern SemaphoreHandle_t spiMutex;

bool spiExchange(SPI_TypeDef *SPIx, size_t length, const uint8_t *data_tx,
		uint8_t *data_rx);
void spiBeginTransaction();
void spiEndTransaction();

#endif /* ATHENA_DECK_HAL_INC_SPI_DRV_H_ */
