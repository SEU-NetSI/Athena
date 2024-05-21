/*
 * uart_hal.h
 *
 *  Created on: May 19, 2024
 *      Author: mfxjx
 */

#ifndef ATHENA_DECK_HAL_INC_UART_HAL_H_
#define ATHENA_DECK_HAL_INC_UART_HAL_H_
#include <stdint.h>


void UART_DMA_Transmit(uint8_t *data, uint32_t length);

#endif /* ATHENA_DECK_HAL_INC_UART_HAL_H_ */
