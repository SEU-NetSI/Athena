/*
 * uart_receive.h
 *
 *  Created on: Jun 15, 2024
 *      Author: mfxjx
 */

#ifndef SYSDRIVER_INC_UART_RECEIVE_H_
#define SYSDRIVER_INC_UART_RECEIVE_H_

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

extern QueueHandle_t UartRxQueue;
extern QueueHandle_t UART2RxQueue;
extern SemaphoreHandle_t uartReadySemaphore;
extern SemaphoreHandle_t UartRxReady;

void CreateUart2RxQueue(void);
void CreateUart3RxQueue(void);
void UARTSysServerInit(void);

void UartRxCallback(void);


#endif /* SYSDRIVER_INC_UART_RECEIVE_H_ */
