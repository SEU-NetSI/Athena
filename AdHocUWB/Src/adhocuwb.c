#include <stdint.h>
#include <string.h>
#include <stdbool.h>


#include "adhocuwb.h"
#include "adhocuwb_impl.h"

#ifndef UWB_DEBUG_ENABLE
  #undef DEBUG_PRINT
  #define DEBUG_PRINT
#endif

static uint16_t MY_UWB_ADDRESS;

static TaskHandle_t adhocuwbTxTaskHandle = 0;

static SemaphoreHandle_t irqSemaphore;

static QueueHandle_t txQueue;
static xQueueHandle queues[UWB_MESSAGE_TYPE_COUNT];
static UWB_Message_Listener_t listeners[UWB_MESSAGE_TYPE_COUNT];


uint16_t uwbGetAddress() {
  return MY_UWB_ADDRESS;
}

int uwbSendPacket(UWB_Packet_t *packet) {
  ASSERT(packet);
  return xQueueSend(txQueue, packet, 0);
}

int uwbSendPacketBlock(UWB_Packet_t *packet) {
  ASSERT(packet);
  return xQueueSend(txQueue, packet, portMAX_DELAY);
}

int uwbSendPacketWait(UWB_Packet_t *packet, int wait) {
  ASSERT(packet);
  return xQueueSend(txQueue, packet, M2T(wait));
}

int uwbReceivePacket(UWB_MESSAGE_TYPE type, UWB_Packet_t *packet) {
  ASSERT(packet);
  ASSERT(type < UWB_MESSAGE_TYPE_COUNT);
  return xQueueReceive(queues[type], packet, 0);
}

int uwbReceivePacketBlock(UWB_MESSAGE_TYPE type, UWB_Packet_t *packet) {
  ASSERT(packet);
  ASSERT(type < UWB_MESSAGE_TYPE_COUNT);
  return xQueueReceive(queues[type], packet, portMAX_DELAY);
}

int uwbReceivePacketWait(UWB_MESSAGE_TYPE type, UWB_Packet_t *packet, int wait) {
  ASSERT(packet);
  ASSERT(type < UWB_MESSAGE_TYPE_COUNT);
  return xQueueReceive(queues[type], packet, M2T(wait));
}

void uwbRegisterListener(UWB_Message_Listener_t *listener) {
  ASSERT(listener->type < UWB_MESSAGE_TYPE_COUNT);
  queues[listener->type] = listener->rxQueue;
  listeners[listener->type] = *listener;
}

void adhocuwb_txCallback(void *parameters) {
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	vTaskNotifyGiveFromISR(adhocuwbTxTaskHandle, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

}

/* Packet dispatcher */
void adhocuwb_rxCallback(void *parameters) {
  UWB_Packet_t *packet = (UWB_Packet_t *) parameters;
  UWB_MESSAGE_TYPE msgType = packet->header.type;

  ASSERT(msgType < UWB_MESSAGE_TYPE_COUNT);

  if (!(packet->header.destAddress == MY_UWB_ADDRESS || packet->header.destAddress == UWB_DEST_ANY)) {
    return;
  }

  if (listeners[msgType].rxCb) {
    listeners[msgType].rxCb(packet);
  }

  if (listeners[msgType].rxQueue) {
	  adhocuwb_xQueueSendFromISR(listeners[msgType].rxQueue, packet);
  }
}

static void adhocuwbTxTask(void *parameters) {
  systemWaitStart();

  UWB_Packet_t packetCache;
  while (true) {
    if (xQueueReceive(txQueue, &packetCache, portMAX_DELAY)) {
      ASSERT(packetCache.header.type < UWB_MESSAGE_TYPE_COUNT);
      ASSERT(packetCache.header.length <= UWB_FRAME_LEN_MAX);

      if (adhocuwb_hdw_send(&packetCache, packetCache.header.length)) {
          if (ulTaskNotifyTake(pdTRUE, M2T(20)) == pdFALSE) {
            DEBUG_PRINT("uwbTxTask: Timeout when waiting for tx success signal from txCallback, os may extremely busy now.\n");
          } else {
            /* Invoke txCallback */
            if (listeners[packetCache.header.type].txCb) {
              listeners[packetCache.header.type].txCb(&packetCache);
            }
          }
      } else {
          DEBUG_PRINT("uwbTxTask:  TX ERROR\n");
      }
      vTaskDelay(M2T(1));   //TODO: test to see if we can remove this delay
    }
  }
}


static void adhocuwbInit() {
  txQueue = xQueueCreate(UWB_TX_QUEUE_SIZE, UWB_TX_QUEUE_ITEM_SIZE);
  xTaskCreate(adhocuwbTxTask, ADHOC_UWB_TX_TASK_NAME, UWB_TASK_STACK_SIZE, NULL,
              ADHOC_UWB_TASK_PRI, &adhocuwbTxTaskHandle);
#ifdef UWB_RANGING_ENABLE
  rangingInit();
#endif
#ifdef UWB_ROUTING_ENABLE
  routingInit();
#endif
#ifdef UWB_RAFT_ENABLE
  raftInit();
#endif
#ifdef UWB_FLOODING_ENABLE
  floodingInit();
#endif
}
