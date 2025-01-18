#ifndef TOF_GET_DATA_H
#define TOF_GET_DATA_H

#include <stdint.h>  // 确保包含标准整数类型

typedef struct tof_packet {
    uint8_t id;
    int16_t distance[64];
} tof_packet;

typedef void (*ToFdataCallback)(const tof_packet* pk);

void tof_get_data(void* argument);
void ToFRegisterMessageHandler(ToFdataCallback callback);

#endif // TOF_GET_DATA_H
