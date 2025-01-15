#include "uwb_send_recv_packet_example.h"

osThreadId_t uwbTaskHandle;
const osThreadAttr_t uwbTask_attributes = {
	  .name = "uwbTask",
	  .stack_size =  2 * UWB_FRAME_LEN_MAX * sizeof(StackType_t), //TODO: check whether this works
	  .priority = (osPriority_t) osPriorityNormal,
	};
static osThreadId_t uwbISRTaskHandle;

static void initUWBConfig(adhocuwb_hdw_cb_t txCb, adhocuwb_hdw_cb_t rxCb){

	// reset dw3000 chip
	dwt_ops.reset(); // this is not necessary

	// prepare the interrupt service routines task
	uwbISRTaskHandle = osThreadNew(uwbISRTask, NULL, &uwbTask_attributes);
	vTaskDelay(100); // wait for the uwbISRTask to start to handle ISR

	// init the dw3000 chip, get ready to rx and rx，下面两次初始化是为了两个不同SPI配置的dw3000设备
	int result = dw3000_init();
	uint32_t dev_id = dwt_readdevid();
	if (dev_id != 0x0 && dev_id != (0xDECA0302))
	{
	  MX_SPI2_Init_IO2IO3();
	  dw3000_init();
	}
	// set the tx and rx callback functions
	adhocuwb_set_hdw_cbs(txCb, rxCb);

	// set the chip in listening mode, rxcallback should be invoked once a packet is received.
	// you should see the RX led flashes at the UWB Deck
	adhocuwb_hdw_force_rx();
}

void simpleTxCallback(void *argument) {	// 发送完数据包后的回调函数
	return;
}

void simpleRxCallback(void *argument) {	// 接收到数据包时的回调函数
	int *packet = (int *) argument;
	int a = packet[1];
	int b = packet[2];
	return;
}

void uwbSendRecvPacketTask(void *argument)
{

	initUWBConfig(simpleTxCallback, simpleRxCallback);


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
