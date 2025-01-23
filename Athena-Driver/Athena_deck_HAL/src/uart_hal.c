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

	LL_DMA_ConfigAddresses(DMA1, LL_DMA_CHANNEL_2,
	                       (uint32_t)data, // data为需要发送的数据数组的地址
	                       LL_USART_DMA_GetRegAddr(USART3, LL_USART_DMA_REG_DATA_TRANSMIT),
	                       LL_DMA_DIRECTION_MEMORY_TO_PERIPH);

	LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_2, length);

    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_2);

    LL_USART_EnableDMAReq_TX(USART3);

    osDelay(5);

    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_2);

}

void UART_DMA_Transmit_1(uint8_t *data, uint32_t length) {

	LL_DMA_ConfigAddresses(DMA2, LL_DMA_CHANNEL_6,
	                       (uint32_t)data, // data为需要发送的数据数组的地址
	                       LL_USART_DMA_GetRegAddr(USART1, LL_USART_DMA_REG_DATA_TRANSMIT),
	                       LL_DMA_DIRECTION_MEMORY_TO_PERIPH);

	LL_DMA_SetDataLength(DMA2, LL_DMA_CHANNEL_6, length);

    LL_DMA_EnableChannel(DMA2, LL_DMA_CHANNEL_6);

    LL_USART_EnableDMAReq_TX(USART1);

    osDelay(5);

    LL_DMA_DisableChannel(DMA2, LL_DMA_CHANNEL_6);

}

void UART_DMA_Receive(uint8_t *data, uint32_t length){

	LL_DMA_ConfigAddresses(DMA1, LL_DMA_CHANNEL_3,
						   LL_USART_DMA_GetRegAddr(USART3, LL_USART_DMA_REG_DATA_RECEIVE),
						   (uint32_t)data,
		                   LL_DMA_DIRECTION_PERIPH_TO_MEMORY);

	LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_3, length);

	LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_3);

	LL_USART_EnableDMAReq_TX(USART3);

	osDelay(10);

	LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_3);

}

void UART2_Transmit(uint8_t *data, uint32_t length){
    while (length > 0) {
        while (!LL_USART_IsActiveFlag_TC(USART2));
        LL_USART_TransmitData8(USART2,(*data & (uint8_t)0xff));
        data++;
        length--;
    }
}
