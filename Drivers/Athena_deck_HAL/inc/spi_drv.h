/*
 * spi_drv.h
 *
 *  Created on: Apr 30, 2024
 *      Author: Yunfan, Hanjie
 */

#ifndef ATHENA_DECK_HAL_INC_SPI_DRV_H_
#define ATHENA_DECK_HAL_INC_SPI_DRV_H_

#include "stm32l4xx.h"
#include "stm32l4xx_ll_spi.h"
#include "stm32l4xx_ll_dma.h"

int spi_transmit(SPI_TypeDef* spix, uint8_t data_in);
int spi_receive(SPI_TypeDef* spix, uint8_t *data_out);
int spi_transmit_receive(SPI_TypeDef* spix, uint8_t data_in, uint8_t *data_out);

#endif /* ATHENA_DECK_HAL_INC_SPI_DRV_H_ */
