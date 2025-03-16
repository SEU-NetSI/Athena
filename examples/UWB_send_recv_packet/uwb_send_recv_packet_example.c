/*
 * uwb_send_recv_packet_example.c
 *
 *  Created on: Mar 15, 2025
 *      Author: twinhorse
 */
#include "dw3000_cbll.h"
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "syslaunch.h"

osThreadId_t uwbSendRecvPacketTaskHandle;

const osThreadAttr_t uwbSendRecvPacketTaskAttributes = {
	  .name = "uwbSendRecvPacketTask",
	  .stack_size =  2 * UWB_FRAME_LEN_MAX * sizeof(StackType_t), //TODO: check whether this works
	  .priority = (osPriority_t) osPriorityNormal,
};

void simpleTxCallback(void *argument) {	// 发送完数据包后的回调函数
	return;
}

void simpleRxCallback(void *argument) {	// 接收到数据包时的回调函数
	uint32_t *packet = (uint32_t *) argument;
	uint32_t a = packet[0];
	uint32_t b = packet[1];
	printf("%ld,\t%lX\n",a,b);
	return;
}

void uwbSendRecvPacketTask(void *argument)
{
    while(!getInitStatus()){
    	vTaskDelay(10);
    }
	adhocuwb_set_hdw_cbs(simpleTxCallback, simpleRxCallback);

	/*============ the above code need only support from BSP/Components/DW3000 =============*/
//	int uwbdata_tx[10] = {1,2,3,4,5,6,7,8,9};
	static uint32_t uwbdata_tx[2] = {1,0x5E02E751};
	// loop forever
	while(1)
	{
	  uwbdata_tx[0]++;
	  adhocuwb_hdw_send(uwbdata_tx, 8);
      vTaskDelay(2000);
	}
}

static void uwb_send_recv_packet_init(){
	uwbSendRecvPacketTaskHandle = osThreadNew(uwbSendRecvPacketTask, NULL, &uwbSendRecvPacketTaskAttributes);
}

static const UserInit uwb_send_recv_packet_struct = {
		.init = uwb_send_recv_packet_init,
};

//USER_INIT(uwb_send_recv_packet_struct);

