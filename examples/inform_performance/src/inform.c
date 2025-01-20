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

static void informHighPerformanceTask(void *argument);

osThreadId_t informHighPerformanceTaskHandle;

const osThreadAttr_t informHighPerformanceTask_attributes = {
  .name = "informHighPerformanceTask",
  .stack_size = 128 * 4,
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
	uint8_t regAddress = 0xBB;
	uint8_t writeByte = 0x59;
	uint8_t regAdd = 0xBC;
	uint8_t datawrite[16];
	for(int i = 0; i < 16; ++i) {
		datawrite[i] = i;
	}
	uint8_t datareceive[16];
	uint8_t readByte;

	for(;;){
		EnableChannelA();
		WriteBytesToFM25xxx(&fm25, regAdd, datawrite, 16);
		osDelay(2000);
		ReadBytesFromFM25xxx(&fm25, regAdd, datareceive, 16);
		EnableChannelB();
	}
}
