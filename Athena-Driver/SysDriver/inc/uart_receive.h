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
extern SemaphoreHandle_t UartRxReady;

void CreateUartRxQueue(void);
void UartRxCallback(void);


#endif /* SYSDRIVER_INC_UART_RECEIVE_H_ */
