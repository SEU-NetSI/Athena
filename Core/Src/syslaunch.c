#include "syslaunch.h"
#include "main.h"

#ifdef ENABLE_UWB_DW3000

#include "libdw3000.h"
#include "dw3000.h"
#include "dwTypes.h"
#include "dw3000_cbll.h"
#include "adhocuwb.h"
#include "uwb_send_print.h"

SemaphoreHandle_t spiDeckTxComplete = NULL;
SemaphoreHandle_t spiDeckRxComplete = NULL;
SemaphoreHandle_t spiDeckMutex = NULL;
SemaphoreHandle_t uwbIrqSemaphore = NULL;

osThreadId_t uwbISRTaskHandle;

const osThreadAttr_t uwbLaunchTaskAttributes = {
		.name = "uwbLaunchTask",
		.stack_size = 2 * UWB_FRAME_LEN_MAX * sizeof(StackType_t),
		.priority = (osPriority_t) osPriorityNormal,
};

const osThreadAttr_t uwbISRTaskAttributes = {
		.name = "uwbISRTask",
		.stack_size = 2 * UWB_FRAME_LEN_MAX * sizeof(StackType_t),
		.priority = (osPriority_t) osPriorityNormal,
};

int spi_deck_init(void)
{
  spiDeckTxComplete = xSemaphoreCreateBinary();
  spiDeckRxComplete = xSemaphoreCreateBinary();
  spiDeckMutex = xSemaphoreCreateMutex();
  uwbIrqSemaphore = xSemaphoreCreateMutex();

	if (spiDeckTxComplete == NULL || spiDeckRxComplete == NULL || spiDeckMutex == NULL || uwbIrqSemaphore == NULL)
	{
	    while (1);
	}

	return 0;
}

static void initUWBConfig(){

	// reset dw3000 chip
	dwt_ops.reset(); // this is not necessary

	// prepare the interrupt service routines task
	uwbISRTaskHandle = osThreadNew(uwbISRTask, NULL, &uwbISRTaskAttributes);
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
	initUWBDebugPrint();
}

static int initStatus;

int getInitStatus(){
	return initStatus;
}

static void uwbLaunchTask(){
	spi_deck_init();
	initUWBConfig();
	vTaskDelay(100);
	adhocuwbInit();
	initStatus = 1;
	while (1) {
		vTaskDelay(1000);
	}
}

static osThreadId_t uwbLaunchTaskHandle;

static void uwb_launch_init(){
	uwbLaunchTaskHandle = osThreadNew(uwbLaunchTask, NULL, &uwbLaunchTaskAttributes);
}

static const UserInit uwb_launch_struct = {
		.init = uwb_launch_init,
};

USER_INIT(uwb_launch_struct);
#endif
