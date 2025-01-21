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
	FM25ObjectType fm25;
	FramArbitrationInit();
	EnableTmux();
	EnableChannelA();
	Fm25cxxInitialization(&fm25,
						  FM25CL64B,
				          ReadDataFromFM25,
				          WriteDataToFM25,
				          LL_mDelay,
				          ChipSelectForFM25
				          );
	EnableChannelB();

	uint8_t regAdd = 0x00;
	uint8_t datawrite[512];
	for(int i = 0; i < 512; ++i) {
		datawrite[i] = i%255;
	}
	uint8_t datareceive[512];

	for(;;){
		EnableChannelA();
		WriteBytesToFM25xxx(&fm25, regAdd, datawrite, 512);
		DEBUG_PRINTF("this is a test: %s \n","hello world");
		osDelay(2000);
		ReadBytesFromFM25xxx(&fm25, regAdd, datareceive, 512);
		EnableChannelB();
	}
}
