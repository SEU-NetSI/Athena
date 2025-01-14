#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

osThreadId_t uwb_get_distances;

const osThreadAttr_t uwb_get_distances_attributes = {
		.name = "uwbGetDistances",
		.stack_size = 2 * UWB_FRAME_LEN_MAX * sizeof(StackType_t),
		.priority = (osPriority_t) osPriorityNormal,
};

static void initUWBConfig(){

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

	// set the chip in listening mode, rxcallback should be invoked once a packet is received.
	// you should see the RX led flashes at the UWB Deck
	adhocuwb_hdw_force_rx();
}


static void uwb_get_distances_init(){
	initUWBConfig();
	vTaskDelay(100);
	adhocuwbInit();
}

static const UserInit uwb_distances_init = {
		.init = uwb_get_distances_init,
};

USER_INIT(uwb_distances_init);
