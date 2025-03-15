#include "fm25cl64.h"
#include "fm25_platform.h"
#include "tmux1574.h"
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "debug.h"
#include "arbitration_fram.h"
#include "fram_sys_xfer.h"
//#include "debug_print.h"
#include "dwTypes.h"
#include "dw3000_cbll.h"
#include "spi.h"


SemaphoreHandle_t spiMutex = NULL;

SemaphoreHandle_t FRAMxferMutex = NULL;

static void informTask(void *argument);

osThreadId_t informTaskHandle;

const osThreadAttr_t informTask_attributes = {
  .name = "informTask",
  .stack_size = 2 * UWB_FRAME_LEN_MAX * sizeof(StackType_t),
  .priority = (osPriority_t) osPriorityNormal,
};

void user_informTask_init(){
	informTaskHandle = osThreadNew(informTask, NULL, &informTask_attributes);
}

static const UserInit informTask_init = {
	.init = user_informTask_init,
	.name = "informH7",
};

//USER_INIT(informTask_init);
extern int ledcode;

static void informTask(void *argument)
{

	// reset dw3000 chip
	dwt_ops.reset(); // this is not necessary

	// prepare the interrupt service routines task
//	uwbISRTaskHandle = osThreadNew(uwbISRTask, NULL, &uwbISRTaskAttributes);
	vTaskDelay(100); // wait for the uwbISRTask to start to handle ISR

	// init the dw3000 chip, get ready to rx and rx，下面两次初始化是为了两个不同SPI配置的dw3000设备
	int result = dw3000_init();
	uint32_t dev_id = dwt_readdevid();
	ledcode=0;
	if (dev_id != 0x0 && dev_id != (0xDECA0302))
	{
		ledcode=(0x1UL << 1U);
		MX_SPI2_Alt_Init();
	}
	adhocuwb_hdw_force_rx();

	while(1){
		osDelay(500);
	}
}
