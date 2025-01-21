/*
 * arbitration_fram.h
 *
 *  Created on: Jan 20, 2025
 *      Author: mfxjx
 */

#ifndef SYSDRIVER_INC_FRAMXFER_ARBITRATION_FRAM_H_
#define SYSDRIVER_INC_FRAMXFER_ARBITRATION_FRAM_H_

#include "spi_drv.h"
#include "main.h"
#include "stm32l4xx_ll_gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "semphr.h"
#include "queue.h"


#define MAX_LOCK_TIME 3

extern SemaphoreHandle_t FramSwitchMutex;

void FramArbitrationInit();

#endif /* SYSDRIVER_INC_FRAMXFER_ARBITRATION_FRAM_H_ */
