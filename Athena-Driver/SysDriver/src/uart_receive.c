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
#include "fifo_buff.h"
#include "debug.h"
#include "stm32l4xx_it.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

#define QUEUE_LENGTH 512
#define ITEM_SIZE sizeof(uint8_t)

QueueHandle_t UART2RxQueue = NULL;
QueueHandle_t UartRxQueue = NULL;
SemaphoreHandle_t uartReadySemaphore = NULL;
SemaphoreHandle_t UartRxReady = NULL;
SemaphoreHandle_t Uart2RxComplete = NULL;
fifo_rx_def *pfifo_x = &fifo_usart_rx_2;
static void UART_ServiceTask();

osThreadId_t UART_ServiceTaskHandle;

const osThreadAttr_t UART_ServiceTask_attributes = {
  .name = "UART_ServiceTask",
  .stack_size = 128 * 15,
  .priority = (osPriority_t) osPriorityNormal,
};

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
	Uart2RxComplete = xSemaphoreCreateBinary();
	UartRxReady = xSemaphoreCreateBinary();
}
void UART_ServiceTask_init(){
	UART_ServiceTaskHandle = osThreadNew(UART_ServiceTask, NULL, &UART_ServiceTask_attributes);
}

static const UserInit uart_sysdriver_init = {
		.init = UART_ServiceTask_init,
};

USER_INIT(uart_sysdriver_init);


static void UART_ServiceTask(){
	for(int i =0; i <10; ++i) {
		LL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
		osDelay(500);
	}
	UARTSysServerInit();
	while(1)
	{
		if(xSemaphoreTake(Uart2RxComplete, pdMS_TO_TICKS(50)) == pdPASS)
	    {
	    uint8_t ucRawData[64];
	    uint16_t usLen = fifo_read_buff(pfifo_x, ucRawData, sizeof(ucRawData));
	    DEBUG_PRINTF("usLen:%d\n", usLen);
	    }
	}
}




