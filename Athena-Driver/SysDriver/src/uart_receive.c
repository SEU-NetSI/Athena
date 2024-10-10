/*
 * uart_receive.c
 *
 *  Created on: Jun 15, 2024
 *      Author: mfxjx
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "uart_hal.h"
#include "uart_receive.h"
#include "stm32l4xx_ll_gpio.h"

#define QUEUE_LENGTH 1024
#define ITEM_SIZE sizeof(uint8_t)

QueueHandle_t UartRxQueue = NULL;
float para[4];

void CreateUartRxQueue(void) {
    UartRxQueue = xQueueCreate(QUEUE_LENGTH, ITEM_SIZE);
    if (UartRxQueue == NULL) {
        while(1);
    }
}

void UartRxCallback(void){
//	int state;
//	xQueueReceive(UartRxQueue, &state, portMAX_DELAY);
//	  switch(state)
//	  {
//	  case 0:
//	  {
//		  para[0] = 0.0;
//		  para[1] = 0.0;
//		  para[2] = 1.0;
//		  para[3] = 0.0;
//		  LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_9);
//		  LL_mDelay(100);
//		  UART_DMA_Transmit((uint8_t *)para, 16);
//		  LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_9);
//		  LL_mDelay(100);
//		  break;
//	  }
//	  case 1:
//	  {
//		  para[0] = 0.1;
//		  para[1] = 0.0;
//		  para[2] = 1.0;
//		  para[3] = 0.0;
//		  LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_9);
//		  LL_mDelay(100);
//		  UART_DMA_Transmit((uint8_t *)para, 16);
//		  LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_9);
//		  LL_mDelay(100);
//		  break;
//	  }
//	  case 2:
//	  {
//		  para[0] = 0.0;
//		  para[1] = 0.1;
//		  para[2] = 1.0;
//		  para[3] = 0.0;
//		  LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_9);
//		  LL_mDelay(100);
//		  UART_DMA_Transmit((uint8_t *)para, 16);
//		  LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_9);
//		  LL_mDelay(100);
//		  break;
//	  }
//	  case 3:
//	  {
//		  para[0] = -0.1;
//		  para[1] = 0.0;
//		  para[2] = 1.0;
//		  para[3] = 0.0;
//		  LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_9);
//		  LL_mDelay(100);
//		  UART_DMA_Transmit((uint8_t *)para, 16);
//		  LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_9);
//		  LL_mDelay(100);
//		  break;
//	  }
//	  case 4:
//	  {
//		  para[0] = 0.0;
//		  para[1] = -0.1;
//		  para[2] = 1.0;
//		  para[3] = 0.0;
//		  LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_9);
//		  LL_mDelay(100);
//		  UART_DMA_Transmit((uint8_t *)para, 16);
//		  LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_9);
//		  LL_mDelay(100);
//		  break;
//	  }
//	  case 5:
//	  {
//		  for(int i=0;i<4;i++)
//		  {
//			  para[i] = 0.0;
//		  }
//		  LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_9);
//		  LL_mDelay(100);
//		  UART_DMA_Transmit((uint8_t *)para, 16);
//		  LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_9);
//		  LL_mDelay(100);
//		  break;
//	  }
//	  }
	xSemaphoreGiveFromISR(UartRxReady, NULL);
}


