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


SemaphoreHandle_t txComplete = NULL;
SemaphoreHandle_t rxComplete = NULL;
SemaphoreHandle_t spiMutex = NULL;
SemaphoreHandle_t FRAMxferMutex = NULL;

static void informTask(void *argument);

osThreadId_t informTaskHandle;

const osThreadAttr_t informTask_attributes = {
  .name = "informTask",
  .stack_size = 128 * 10,
  .priority = (osPriority_t) osPriorityNormal,
};

void user_informTask_init(){
	informTaskHandle = osThreadNew(informTask, NULL, &informTask_attributes);
}

static const UserInit informTask_init = {
	.init = user_informTask_init,
	.name = "informH7",
};

USER_INIT(informTask_init);

static void informTask(void *argument)
{
	LL_GPIO_SetOutputPin(LED_GPIO_Port, LED_Pin);
	txComplete = xSemaphoreCreateBinary();
	rxComplete = xSemaphoreCreateBinary();
	spiMutex = xSemaphoreCreateMutex();
//	FRAMxferMutex = xSemaphoreCreateMutex();

	Framinit();
	LL_GPIO_ResetOutputPin(LED_GPIO_Port, LED_Pin);
	uint8_t data[100] = {0};
	uint8_t data1[100] = {0};
	for(int i = 0; i < 100; ++i) {
		data[i] = i;
	}
	DataPacket pk;
	pk.length = 100;
	pk.type = 1;
	pk.content = malloc(pk.length);
	if (pk.content != NULL) {
	    memcpy(pk.content, data, pk.length);
	}
	while(1){
//		XfertoPerformance(&pk);
		ReadBytesFromFM25xxx(&fm25,0x00,data1,100);
		LL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
		osDelay(1000);
	}
}
