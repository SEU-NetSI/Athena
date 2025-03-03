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
#include "main.h"
#include "uart_hal.h"
#include "uart_receive.h"

#define QUEUE_LENGTH 512
#define ITEM_SIZE sizeof(uint8_t)

QueueHandle_t UART2RxQueue = NULL;
QueueHandle_t UartRxQueue = NULL;
SemaphoreHandle_t uartReadySemaphore = NULL;
SemaphoreHandle_t UartRxReady = NULL;

void CreateUart3RxQueue(void) {
    UartRxQueue = xQueueCreate(QUEUE_LENGTH, ITEM_SIZE);
    if (UartRxQueue == NULL) {
        while(1);
    }
}

void CreateUart2RxQueue(void) {
	UART2RxQueue = xQueueCreate(QUEUE_LENGTH, ITEM_SIZE);
    if (UartRxQueue == NULL) {
        while(1);
    }
}

void UartRxCallback(void){
	xSemaphoreGiveFromISR(UartRxReady, NULL);
}

void UARTSysServerInit(void) {
	CreateUart3RxQueue();
	CreateUart2RxQueue();
	uartReadySemaphore = xSemaphoreCreateBinary();
	UartRxReady = xSemaphoreCreateBinary();
}

static const UserInit uart_sysdriver_init = {
		.init = UARTSysServerInit,
};

USER_INIT(uart_sysdriver_init);



