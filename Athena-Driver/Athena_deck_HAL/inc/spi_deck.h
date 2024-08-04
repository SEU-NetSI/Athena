/*
 * spi_deck.h
 *
 *  Created on: Aug 4, 2024
 *      Author: Feng Shan
 */

#ifndef ATHENA_DECK_HAL_INC_SPI_DECK_H_
#define ATHENA_DECK_HAL_INC_SPI_DECK_H_

#include <stdbool.h>
#include "stm32l4xx.h"
#include "stm32l4xx_ll_spi.h"
#include "stm32l4xx_ll_dma.h"
#include "FreeRTOS.h"
#include "semphr.h"


bool spiDeckExchange(size_t length, const uint8_t * data_tx, uint8_t * data_rx);
void spiDeckBeginTransaction();
void spiDeckEndTransaction();

#endif /* ATHENA_DECK_HAL_INC_SPI_DECK_H_ */
