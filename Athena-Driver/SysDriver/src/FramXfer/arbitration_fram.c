/*
 * arbitration_fram.c
 *
 *  Created on: Jan 20, 2025
 *      Author: mfxjx
 */

#include "arbitration_fram.h"
#include "tmux1574.h"
#include "spi_drv.h"
#include "main.h"
#include "stm32l4xx_ll_gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "semphr.h"
#include "queue.h"

#define MAX_LOCK_TIME 4
QueueHandle_t FramRollbackQueue = NULL;
SemaphoreHandle_t FramSwtichMutex = NULL;
/*
 * @brief high performance unit must use FRAM now.
 * @steps
 * 1. switch the control of FRAM
 * 2. lock the FRAM control switch during next max_lock_time
 * 3. notify FRAM system server layer not to switch
 * 4. roll the data
 * */
void FramArbitrationCallback(){
	if(FramSwtichMutex){
		LL_GPIO_SetOutputPin(TMUX_SEL_Port, TMUX_SEL_Pin);
		xSemaphoreTakeFromISR(FramSwtichMutex, portMAX_DELAY);
		vTaskDelay(MAX_LOCK_TIME);
		xSemaphoreGiveFromISR(FramSwtichMutex);
	}
}

void FramArbitrationInit() {
    FramSwtichMutex = xSemaphoreCreateMutex();
    if (FramSwtichMutex == NULL) {
        Error_Handler();
    }
}