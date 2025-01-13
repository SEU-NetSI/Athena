/*
 * cpx.c
 *
 *  Created on: Aug 20, 2024
 *      Author: 18387
 */


#include "cpx/cpx.h"
#include <string.h>
#include <stdbool.h>
#include "uart_hal.h"
#include "cpx/uart_transport.h"
#include "cpx/router.h"
#include "debug.h"
#include "cmsis_os.h"
static cpxAppMessageHandlerCallback_t appMessageHandlerCallback = NULL;
static CPXPacket_t cpxRxPacket;

osThreadId_t cpxTaskHandle;

const osThreadAttr_t cpxTask_attributes = {
  .name = "cpxTask",
  .stack_size = 5000,
  .priority = (osPriority_t) osPriorityNormal,
};
void cpx(void* _param) {
    while (1) {
        // 从内部队列rxq接收数据包
    	cpxInternalRouterReceiveOthers(&cpxRxPacket);
        // 根据目标地址路由消息
        switch (cpxRxPacket.route.function) {
            case CPX_F_APP:
                if (appMessageHandlerCallback) {
                    appMessageHandlerCallback(&cpxRxPacket); // 调用应用层回调处理
                }
                break;


            default:
                DEBUG_PRINTF("Unhandled function: 0x%02X\n", cpxRxPacket.route.function);
                break;
        }
    }
}


void cpxInitRoute(const CPXTarget_t source, const CPXTarget_t destination, const CPXFunction_t function, CPXRouting_t *route) {
    route->source = source;
    route->destination = destination;
    route->function = function;
    route->version = CPX_VERSION;
    route->lastPacket = true;      // 默认设置为最后一个包
}

bool cpxCheckVersion(uint8_t version) {

    return version == CPX_VERSION;
}

void cpxRegisterAppMessageHandler(cpxAppMessageHandlerCallback_t callback) {

    appMessageHandlerCallback = callback;
}


void cpxRouteToPacked(const CPXRouting_t* route, CPXRoutingPacked_t* packed) {
    packed->source = route->source;
    packed->destination = route->destination;
    packed->function = route->function;
    packed->version = route->version;
    packed->lastPacket = route->lastPacket;
}

void cpxPackedToRoute(const CPXRoutingPacked_t* packed, CPXRouting_t* route) {
    if(CPX_VERSION == packed->version)
    {
        route->version = packed->version;
        route->source = packed->source;
        route->destination = packed->destination;
        route->function = packed->function;
        route->lastPacket = packed->lastPacket;
    }
}

void cpxSendPacket(const CPXRoutablePacket_t* packet) {
	uart_transport_send(packet);

}

//void cpxSendPacketBlocking(const CPXPacket_t *packet) {
//    CPXPacketPacked_t txpPacked;
//
//    // 填充数据
//    txpPacked.wireLength = packet->dataLength + CPX_HEADER_SIZE;
//    txpPacked.route.destination = packet->route.destination;
//    txpPacked.route.source = packet->route.source;
//    txpPacked.route.function = packet->route.function;
//    txpPacked.route.version = packet->route.version;
//    txpPacked.route.lastPacket = packet->route.lastPacket;
//    memcpy(txpPacked.data, packet->data, packet->dataLength);
//
//    // 通过 UART 发送数据
//    UART_DMA_Transmit((uint8_t *)&txpPacked, txpPacked.wireLength);
//}

void cpxInit(void) {
	cpxTaskHandle = osThreadNew(cpx, NULL, &cpxTask_attributes);

}


