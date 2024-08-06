/*
 * dw3000deck_ll.h
 *
 *  Created on: Aug 4, 2024
 *      Author: twinhorse
 */

#include "stm32l4xx_ll_gpio.h"
#include "spi_deck.h"


#ifndef BSP_COMPONENTS_DW300_INC_DW3000DECK_LL_H_
#define BSP_COMPONENTS_DW300_INC_DW3000DECK_LL_H_


#define DW3000Deck_CS_GPIO_Port GPIOA
#define DW3000Deck_CS_Pin LL_GPIO_PIN_1

#define DUMMY_BYTE 0x00

extern SemaphoreHandle_t spiDeckTxComplete;
extern SemaphoreHandle_t spiDeckRxComplete;
extern SemaphoreHandle_t spiDeckMutex;

void BSP_DW3000_Read_ID(uint32_t *dw3000ID);


#endif /* BSP_COMPONENTS_DW300_INC_DW3000DECK_LL_H_ */
