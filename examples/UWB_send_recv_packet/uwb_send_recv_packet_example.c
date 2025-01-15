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
	int *packet = (int *) argument;
	int a = packet[0];
	int b = packet[1];
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
	int uwbdata_tx[2] = {1,2};
	// loop forever
	while(1)
	{
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

USER_INIT(uwb_send_recv_packet_struct);
