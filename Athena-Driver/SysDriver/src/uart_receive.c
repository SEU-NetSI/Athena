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


#define QUEUE_LENGTH 1024
#define ITEM_SIZE sizeof(uint8_t)

QueueHandle_t UartRxQueue = NULL;

void CreateUartRxQueue(void) {
    UartRxQueue = xQueueCreate(QUEUE_LENGTH, ITEM_SIZE);
    if (UartRxQueue == NULL) {
        while(1);
    }
}

void UartRxCallback(void){
	xSemaphoreGiveFromISR(UartRxReady, NULL);
}


