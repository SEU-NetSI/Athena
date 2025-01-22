/*
 * inform.c
 *
 *  Created on: Jan 20, 2025
 *      Author: mfxjx
 */

#include "fm25cl64.h"
#include "fm25_platform.h"
#include "inform.h"
#include "tmux1574.h"
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "debug.h"
#include "arbitration_fram.h"
#include "fram_sys_xfer.h"

static void informHighPerformanceTask(void *argument);

osThreadId_t informHighPerformanceTaskHandle;

const osThreadAttr_t informHighPerformanceTask_attributes = {
  .name = "informHighPerformanceTask",
  .stack_size = 128 * 15,
  .priority = (osPriority_t) osPriorityNormal,
};

void user_informHighPerformanceTask_init(){
	informHighPerformanceTaskHandle = osThreadNew(informHighPerformanceTask, NULL, &informHighPerformanceTask_attributes);
}

static const UserInit informHighPerformanceTask_init = {
	.init = user_informHighPerformanceTask_init,
};

USER_INIT(informHighPerformanceTask_init);

static void informHighPerformanceTask(void *argument)
{
	Framinit();
	uint8_t data[100] = {0};
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
		XfertoPerformance(&pk);
		int stackWaterMark = uxTaskGetStackHighWaterMark(informHighPerformanceTaskHandle);
		LL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
		osDelay(1000);
	}
}
