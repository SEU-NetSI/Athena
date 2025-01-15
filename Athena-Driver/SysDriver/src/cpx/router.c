/*
 * router.c
 *
 *  Created on: Dec 30, 2024
 *      Author: 18387
 */


#include "cpx/cpx.h"
#include <string.h>
#include <stdbool.h>
#include "uart_hal.h"
#include "cpx/uart_transport.h"
#include "cpx/router.h"
#include "cpx/cpx.h"
#include "debug.h"
#include "cmsis_os.h"

#define QUEUE_LENGTH (2)
osThreadId_t router_uartTaskHandle;
const osThreadAttr_t router_uartTask_attributes = {
  .name = "router_uartTask",
  .stack_size = 500,
  .priority = (osPriority_t) osPriorityAboveNormal,
};

osThreadId_t router_internalTaskHandle;
const osThreadAttr_t router_internalTask_attributes = {
  .name = "router_internalTask",
  .stack_size = 500,
  .priority = (osPriority_t) osPriorityAboveNormal,
};

typedef struct
{
  CPXRoutablePacket_t txp;
} RouteContext_t;

static RouteContext_t internal_task_context;
static CPXRoutablePacket_t internalRxBuf;

static RouteContext_t uart_task_context;
static CPXRoutablePacket_t uartRxBuf;

typedef void (*Receiver_t)(CPXRoutablePacket_t *packet);
typedef void (*Sender_t)(const CPXRoutablePacket_t *packet);

static const int START_UP_UART_ROUTER_RUNNING = (1 << 0);
static const int START_UP_INTERNAL_ROUTER_RUNNING = (1 << 1);

static EventGroupHandle_t startUpEventGroup;

static xQueueHandle rxq;

static xQueueHandle txq;



void cpxInternalRouterReceiveOthers(CPXPacket_t * packet) {
  xQueueReceive(rxq, packet, (TickType_t)portMAX_DELAY);
}

void cpxInternalRouterRouteIn(const CPXRoutablePacket_t* packet) {
  // this should never fail, as it should be checked when the packet is received
  // however, double checking doesn't harm
  if (cpxCheckVersion(packet->route.version)) {
        xQueueSend(rxq, packet, portMAX_DELAY);
    }

}

void cpxInternalRouterRouteOut(CPXRoutablePacket_t* packet) {
  xQueueReceive(txq, packet, (TickType_t)portMAX_DELAY);
}

static void splitAndSend(const CPXRoutablePacket_t *rxp, RouteContext_t *context, Sender_t sender, const uint16_t mtu)
{
  CPXRoutablePacket_t *txp = &context->txp;

  txp->route = rxp->route;

  uint16_t remainingToSend = rxp->dataLength;
  const uint8_t *startOfDataToSend = rxp->data;
  while (remainingToSend > 0)
  {
    uint16_t toSend = remainingToSend;
    bool lastPacket = rxp->route.lastPacket;
    if (toSend > mtu)
    {
      toSend = mtu;
      lastPacket = false;
    }

    memcpy(txp->data, startOfDataToSend, toSend);
    txp->dataLength = toSend;
    txp->route.lastPacket = lastPacket;
    sender(txp);

    remainingToSend -= toSend;
    startOfDataToSend += toSend;
  }
}

static void route(Receiver_t receive, CPXRoutablePacket_t *rxp, RouteContext_t *context, const char *routerName)
{
  while (1)
  {
    receive(rxp);
    // this should never fail, as it should be checked when the packet is received
    // however, double checking doesn't harm
    if (cpxCheckVersion(rxp->route.version))
    {
      const CPXTarget_t source = rxp->route.source;
      const CPXTarget_t destination = rxp->route.destination;
      const uint16_t cpxDataLength = rxp->dataLength;

      switch (destination)
      {
      case CPX_T_WIFI_HOST:
      case CPX_T_ESP32:

      case CPX_T_STM32:
        // DEBUG_PRINT("%s [0x%02X] -> UART2 [0x%02X] (%u)\n", routerName, source, destination, cpxDataLength);
        splitAndSend(rxp, context, uart_transport_send, UART_TRANSPORT_MTU - CPX_HEADER_SIZE);
        break;
      case CPX_T_GAP8:
        // DEBUG_PRINT("%s [0x%02X] -> STM32 [0x%02X] (%u)\n", routerName, source, destination, cpxDataLength);
        splitAndSend(rxp, context, cpxInternalRouterRouteIn, UART_TRANSPORT_MTU - CPX_HEADER_SIZE);
        break;
      default:
        DEBUG_PRINTF("Cannot route from %s [0x%02X] to [0x%02X](%u)\n", routerName, source, destination, cpxDataLength);
        break;
      }
    }
  }
}

static void router_from_uart(void *_param)
{
  xEventGroupSetBits(startUpEventGroup, START_UP_UART_ROUTER_RUNNING);
  route(uart_transport_receive, &uartRxBuf, &uart_task_context, "UART2");
}

static void router_from_internal(void *_param)
{
  xEventGroupSetBits(startUpEventGroup, START_UP_INTERNAL_ROUTER_RUNNING);
  route(cpxInternalRouterRouteOut, &internalRxBuf, &internal_task_context, "Athena");
}

void routerInit()
{
	txq = xQueueCreate(QUEUE_LENGTH, sizeof(CPXPacket_t));

	  rxq = xQueueCreate(QUEUE_LENGTH, sizeof(CPXPacket_t));;
  startUpEventGroup = xEventGroupCreate();
  xEventGroupClearBits(startUpEventGroup, START_UP_UART_ROUTER_RUNNING | START_UP_INTERNAL_ROUTER_RUNNING);

  router_uartTaskHandle = osThreadNew(router_from_uart, NULL, &router_uartTask_attributes);
  //xTaskCreate(router_from_uart, CPX_RT_UART_TASK_NAME, AI_DECK_TASK_STACKSIZE, NULL, AI_DECK_TASK_PRI, NULL);
  //xTaskCreate(router_from_internal, CPX_RT_INT_TASK_NAME, AI_DECK_TASK_STACKSIZE, NULL, AI_DECK_TASK_PRI, NULL);
  router_internalTaskHandle = osThreadNew(router_from_internal, NULL, &router_internalTask_attributes);
  DEBUG_PRINTF("Waiting for CPX External router initialization\n");
  xEventGroupWaitBits(startUpEventGroup,
                      START_UP_UART_ROUTER_RUNNING | START_UP_INTERNAL_ROUTER_RUNNING,
                      pdTRUE, // Clear bits before returning
                      pdTRUE, // Wait for all bits
                      portMAX_DELAY);

  DEBUG_PRINTF("CPX External router initialized, CPX_VERSION: %d\n", CPX_VERSION);
}

void cpxSendPacketBlocking(const CPXPacket_t * packet) {
    xQueueSend(txq, packet, portMAX_DELAY);
}
