/*
 * uart_hal.c
 *
 *  Created on: May 19, 2024
 *      Author: mfxjx
 */

#include "uart_hal.h"
#include "stm32l4xx.h"
#include "stm32l4xx_ll_usart.h"
#include "stm32l4xx_ll_dma.h"
#include <stdio.h>

void UART_DMA_Transmit(uint8_t *data, uint32_t length) {

	LL_DMA_ConfigAddresses(DMA2, LL_DMA_CHANNEL_6,
	                       (uint32_t)data, // data为需要发送的数据数组的地址
	                       LL_USART_DMA_GetRegAddr(USART1, LL_USART_DMA_REG_DATA_TRANSMIT),
	                       LL_DMA_DIRECTION_MEMORY_TO_PERIPH);

	LL_DMA_SetDataLength(DMA2, LL_DMA_CHANNEL_6, length);

    LL_DMA_EnableChannel(DMA2, LL_DMA_CHANNEL_6);

    LL_USART_EnableDMAReq_TX(USART1);

    LL_DMA_DisableChannel(DMA2, LL_DMA_CHANNEL_6);

}
