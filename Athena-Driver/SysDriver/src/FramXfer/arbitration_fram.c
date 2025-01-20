/*
 * arbitration_fram.c
 *
 *  Created on: Jan 20, 2025
 *      Author: mfxjx
 */

#include "arbitration_fram.h"
#include "tmux1574.h"
#include "spi_drv.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "semphr.h"

void FramArbitrationCallback(){
	if(spi1Mutex){
		EnableChannelB();
		xSemaphoreTake(spi1Mutex, portMAX_DELAY);
	}
}
