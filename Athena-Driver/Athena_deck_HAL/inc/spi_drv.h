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

extern SemaphoreHandle_t txComplete;
extern SemaphoreHandle_t rxComplete;
extern SemaphoreHandle_t spiMutex;

bool spiExchange(SPI_TypeDef* SPIx, size_t length, const uint8_t * data_tx, uint8_t * data_rx);
void spiBeginTransaction();
void spiEndTransaction();

#endif /* ATHENA_DECK_HAL_INC_SPI_DRV_H_ */
