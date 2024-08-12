#ifndef __ADHOCUWB_H__
#define __ADHOCUWB_H__

#include "adhocuwb_impl.h"

//#define UWB_DEBUG_ENABLE
//#define UWB_RANGING_ENABLE
//#define UWB_ROUTING_ENABLE
//#define UWB_RAFT_ENABLE
//#define UWB_FLOODING_ENABLE

#define UWB_TASK_STACK_SIZE (2 * UWB_FRAME_LEN_MAX)

/* Queue Constants */
#define UWB_TX_QUEUE_SIZE 5
#define UWB_TX_QUEUE_ITEM_SIZE sizeof(UWB_Packet_t)

/* UWB Packet */
#define UWB_PACKET_SIZE_MAX UWB_FRAME_LEN_MAX
#define UWB_PAYLOAD_SIZE_MAX (UWB_PACKET_SIZE_MAX - sizeof(UWB_Packet_Header_t))
#define UWB_DEST_ANY 65535
#define UWB_DEST_EMPTY 65534

typedef uint16_t UWB_Address_t;
typedef portTickType Time_t;

/* UWB packet definition */
typedef enum {
  UWB_RANGING_MESSAGE = 0,
  UWB_FLOODING_MESSAGE = 1,
  UWB_DATA_MESSAGE = 2,
  UWB_AODV_MESSAGE = 3,
  UWB_OLSR_MESSAGE = 4,
  UWB_MESSAGE_TYPE_COUNT, /* only used for counting message types. */
} UWB_MESSAGE_TYPE;

typedef struct {
  UWB_Address_t srcAddress; // mac address, currently using MY_UWB_ADDRESS
  UWB_Address_t destAddress; // mac address
  UWB_MESSAGE_TYPE type: 6;
  uint16_t length: 10;
} __attribute__((packed)) UWB_Packet_Header_t;

typedef struct {
  UWB_Packet_Header_t header; // Packet header
  uint8_t payload[UWB_PAYLOAD_SIZE_MAX];
} __attribute__((packed)) UWB_Packet_t;

typedef void (*UWBCallback)(void *);

typedef struct {
  UWB_MESSAGE_TYPE type;
  QueueHandle_t rxQueue;
  UWBCallback rxCb;
  UWBCallback txCb;
} UWB_Message_Listener_t;

/* UWB operations */
uint16_t uwbGetAddress();
int uwbSendPacket(UWB_Packet_t *packet);
int uwbSendPacketBlock(UWB_Packet_t *packet);
int uwbSendPacketWait(UWB_Packet_t *packet, int wait);
int uwbReceivePacket(UWB_MESSAGE_TYPE type, UWB_Packet_t *packet);
int uwbReceivePacketBlock(UWB_MESSAGE_TYPE type, UWB_Packet_t *packet);
int uwbReceivePacketWait(UWB_MESSAGE_TYPE type, UWB_Packet_t *packet, int wait);
void uwbRegisterListener(UWB_Message_Listener_t *listener);
void adhocuwbInit();

#endif
