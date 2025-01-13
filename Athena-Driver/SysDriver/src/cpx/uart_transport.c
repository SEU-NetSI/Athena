

#include "cpx/uart_transport.h"

#include <stdio.h>
#include <string.h>
#include "debug.h"
#include "w25q64_ll.h"
#include "cmsis_os.h"
#include "uart_receive.h"
#include "usart.h"
// Length of start + payloadLength
#define UART_HEADER_LENGTH 2
#define UART_CRC_LENGTH 1
#define UART_META_LENGTH (UART_HEADER_LENGTH + UART_CRC_LENGTH)

typedef struct {
    CPXRoutingPacked_t route;
    uint8_t data[UART_TRANSPORT_MTU - CPX_ROUTING_PACKED_SIZE];
} __attribute__((packed)) uartTransportPayload_t;

typedef struct {
    uint8_t start;
    uint8_t payloadLength; // Excluding start and crc
    union {
        uartTransportPayload_t routablePayload;
        uint8_t payload[UART_TRANSPORT_MTU];
    };

    uint8_t crcPlaceHolder; // Not actual position. CRC is added after the last byte of payload
} __attribute__((packed)) uart_transport_packet_t;



#define TX_QUEUE_LENGTH 10
#define RX_QUEUE_LENGTH 10

static xQueueHandle tx_queue;
static xQueueHandle rx_queue;

static CPXRoutablePacket_t qPacket;
static uart_transport_packet_t txp;
static uart_transport_packet_t rxp;

static EventGroupHandle_t evGroup;
static EventGroupHandle_t startUpEventGroup;

osThreadId_t rxTaskHandle;
const osThreadAttr_t rxTask_attributes = {
  .name = "rxTask",
  .stack_size = 2000,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
osThreadId_t txTaskHandle;
const osThreadAttr_t txTask_attributes = {
  .name = "txTask",
  .stack_size = 2000,
  .priority = (osPriority_t) osPriorityNormal,
};
#define CTS_EVENT (1<<0)
#define CTR_EVENT (1<<1)
#define TXQ_EVENT (1<<2)

#define START_UP_RX_RUNNING (1<<0)
#define START_UP_TX_RUNNING (1<<1)

static uint8_t calcCrc(const uart_transport_packet_t* packet) {
  const uint8_t* start = (const uint8_t*) packet;
  const uint8_t* end = &packet->payload[packet->payloadLength];

  uint8_t crc = 0;
  for (const uint8_t* p = start; p < end; p++) {
    crc ^= *p;
  }

  return crc;
}




static void uart_tx_task(void* _param) {
  uint8_t ctr[] = {0xFF, 0x00};
  EventBits_t evBits = 0;
  //LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_9);
  // Note: RX task must be running before we start the TX task
  xEventGroupSetBits(startUpEventGroup, START_UP_TX_RUNNING);

  do {
	  //UART_DMA_Transmit(ctr, sizeof(ctr));
	  Uart3_SendStr((char*)ctr, sizeof(ctr));
	  //LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_9);
	  vTaskDelay(pdMS_TO_TICKS(100));
	  evBits = xEventGroupGetBits(evGroup);
  } while ((evBits & CTS_EVENT) != CTS_EVENT);
  //LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_9);

  while(1) {
    // If we have nothing to send then wait, either for something to be
    // queued or for a request to send CTR
    if (uxQueueMessagesWaiting(tx_queue) == 0) {
    	//DEBUG_PRINTF("waiting for ctr/txq");
    	evBits = xEventGroupWaitBits(evGroup,
                                CTR_EVENT | TXQ_EVENT,
                                pdTRUE, // Clear bits before returning
                                pdFALSE, // Wait for any bit
                                portMAX_DELAY);
      if ((evBits & CTR_EVENT) == CTR_EVENT) {
    	  //DEBUG_PRINTF("send ctr\n");
    	  //UART_DMA_Transmit(ctr, sizeof(ctr));
    	  Uart3_SendStr((char*)ctr, sizeof(ctr));
    	  //LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_9);
      }
    }

    if (uxQueueMessagesWaiting(tx_queue) > 0) {
      // Dequeue and wait for either CTS or CTR
      //LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_9);
      xQueueReceive(tx_queue, &qPacket, 0);
      txp.start = 0xFF;
      txp.payloadLength = qPacket.dataLength + CPX_ROUTING_PACKED_SIZE;
      cpxRouteToPacked(&qPacket.route, &txp.routablePayload.route);
      memcpy(txp.routablePayload.data, qPacket.data, txp.payloadLength);
      txp.payload[txp.payloadLength] = calcCrc(&txp);
      //LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_9);
      do {
    	//DEBUG_PRINTF("waingting for ctr/cts\n");
        evBits = xEventGroupWaitBits(evGroup,
                                CTR_EVENT | CTS_EVENT,
                                pdTRUE, // Clear bits before returning
                                pdFALSE, // Wait for any bit
                                portMAX_DELAY);
        if ((evBits & CTR_EVENT) == CTR_EVENT) {
        	//DEBUG_PRINTF("send ctr\n");
        	//UART_DMA_Transmit(ctr, sizeof(ctr));
        	Uart3_SendStr((char*)ctr, sizeof(ctr));
        }
      } while ((evBits & CTS_EVENT) != CTS_EVENT);
      //DEBUG_PRINTF("send packet\n");
      //UART_DMA_Transmit((uint8_t *)&txp, txp.payloadLength + UART_META_LENGTH);
      Uart3_SendStr((char*)&txp, txp.payloadLength + UART_META_LENGTH);
      //LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_9);
    }
  }
}


static void uart_rx_task(void* _param) {
  xEventGroupSetBits(startUpEventGroup, START_UP_RX_RUNNING);

  while(1) {
    do {
    	xQueueReceive(UartRxQueue, &rxp.start, portMAX_DELAY);
    	//DEBUG_PRINTF("%u\n",rxp.start);
    	//LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_9);
    	vTaskDelay(pdMS_TO_TICKS(10));
    } while (rxp.start != 0xFF);
    xQueueReceive(UartRxQueue, &rxp.payloadLength, portMAX_DELAY);
    //DEBUG_PRINTF("%u\n",rxp.payloadLength);
    LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_9);
    if (rxp.payloadLength == 0) {
    	//DEBUG_PRINTF("received cts\n");
      xEventGroupSetBits(evGroup, CTS_EVENT);
      //LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_9);
    } else {
    	for (int i = 0; i < rxp.payloadLength + UART_CRC_LENGTH; i++) {
    	                xQueueReceive(UartRxQueue, &rxp.payload[i], portMAX_DELAY);
    	            }
      assert (rxp.payload[rxp.payloadLength] == calcCrc(&rxp));
      //DEBUG_PRINTF("received ======\n a packet\n");
      // Post on RX queue and send flow control
      // Optimize a bit here
      if (uxQueueSpacesAvailable(rx_queue) > 0) {
        xEventGroupSetBits(evGroup, CTR_EVENT);
        xQueueSend(rx_queue, &rxp, portMAX_DELAY);
      } else {
        xQueueSend(rx_queue, &rxp, portMAX_DELAY);
        xEventGroupSetBits(evGroup, CTR_EVENT);
      }
    }
  }

}

void uart_transport_init() {
	DEBUG_PRINTF("this is a init test: \n");
    // Setting up synchronization items
    tx_queue = xQueueCreate(TX_QUEUE_LENGTH, sizeof(CPXRoutablePacket_t));
    rx_queue = xQueueCreate(RX_QUEUE_LENGTH, sizeof(uart_transport_packet_t));

    evGroup = xEventGroupCreate();

//    const uart_config_t uart_config = {
//        .baud_rate = 576000,
//        .data_bits = UART_DATA_8_BITS,
//        .parity = UART_PARITY_DISABLE,
//        .stop_bits = UART_STOP_BITS_1,
//        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
//        //.source_clk = UART_SCLK_APB,
//    };
//    // We won't use a buffer for sending data.
//    uart_driver_install(UART_NUM_0, UART_TRANSPORT_MTU * 2, UART_TRANSPORT_MTU * 2, 0, NULL, 0);
//    uart_param_config(UART_NUM_0, &uart_config);
//    uart_set_pin(UART_NUM_0, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    // Launching communication tasks
    startUpEventGroup = xEventGroupCreate();
    xEventGroupClearBits(startUpEventGroup, START_UP_RX_RUNNING | START_UP_TX_RUNNING);
    //xTaskCreate(uart_rx_task, "UART RX transport", 5000, NULL, 1, NULL);
    rxTaskHandle = osThreadNew(uart_rx_task, NULL, &rxTask_attributes);
    //DEBUG_PRINTF("waiting for rx to start\n");
    xEventGroupWaitBits(startUpEventGroup,
                        START_UP_RX_RUNNING,
                        pdTRUE, // Clear bits before returning
                        pdTRUE, // Wait for all bits
                        portMAX_DELAY);

    // We need to hold off here to make sure that the RX task
    // has started up and is waiting for chars before he TX task is started, otherwise we might send
    // CTR and miss CTS (which means that the STM32 will stop sending CTS
    // too early and we cannot sync)
    osDelay(pdMS_TO_TICKS(100));
    //xTaskCreate(uart_tx_task, "UART TX transport", 5000, NULL, 1, NULL);
    txTaskHandle = osThreadNew(uart_tx_task, NULL, &txTask_attributes);
    //DEBUG_PRINTF("waiting for tx to start\n");
    xEventGroupWaitBits(startUpEventGroup,
                        START_UP_TX_RUNNING,
                        pdTRUE, // Clear bits before returning
                        pdTRUE, // Wait for all bits
                        portMAX_DELAY);
    //DEBUG_PRINTF("transport initialized\n");
}

void uart_transport_send(const CPXRoutablePacket_t* packet) {
  assert(packet->dataLength <= UART_TRANSPORT_MTU - CPX_ROUTING_PACKED_SIZE);

  xQueueSend(tx_queue, packet, portMAX_DELAY);
  //DEBUG_PRINTF("send to uart txq\n");
  xEventGroupSetBits(evGroup, TXQ_EVENT);
  //LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_9);
}

void uart_transport_receive(CPXRoutablePacket_t* packet) {
  // Not reentrant safe. Assume only one task dequeues packets
  static uart_transport_packet_t rxp;

  xQueueReceive(rx_queue, &rxp, portMAX_DELAY);

  packet->dataLength = rxp.payloadLength - CPX_ROUTING_PACKED_SIZE;

  cpxPackedToRoute(&rxp.routablePayload.route, &packet->route);

  memcpy(packet->data, rxp.routablePayload.data, packet->dataLength);
  //LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_9);
}
