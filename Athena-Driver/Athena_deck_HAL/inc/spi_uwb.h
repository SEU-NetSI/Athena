/*
 * spi_uwb.h
 *
 *  Created on: Aug 4, 2024
 *      Author: Feng Shan
 */

#ifndef __SPI_UWB_H__
#define __SPI_UWB_H__

#include <stdbool.h>
#include "stm32l4xx.h"
#include "stm32l4xx_ll_spi.h"
#include "stm32l4xx_ll_dma.h"
#include "FreeRTOS.h"
#include "semphr.h"


bool spiUwbExchange(size_t length, const uint8_t * data_tx, uint8_t * data_rx);
void spiUwbBeginTransaction();
void spiUwbEndTransaction();

#endif /* __SPI_UWB_H__ */
