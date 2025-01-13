#pragma once

// The UART transport module represents the transport link between the router and the STM on the Crazyflie.

#include <stdint.h>
#include <stddef.h>
#include "cpx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"
#define UART_TRANSPORT_MTU 100



void uart_transport_init();

// Interface used by the router
void uart_transport_send(const CPXRoutablePacket_t* packet);
void uart_transport_receive(CPXRoutablePacket_t* packet);
