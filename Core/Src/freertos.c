/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "i2c_drv.h"
#include "spi_drv.h"
#include "spi.h"
#include "usart.h"
#include "debug.h"

//#include "adhocuwb.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
SemaphoreHandle_t txComplete = NULL;
SemaphoreHandle_t rxComplete = NULL;
SemaphoreHandle_t spiMutex = NULL;
SemaphoreHandle_t UartRxReady = NULL;

extern const struct user_init * _userInit_start;
extern const struct user_init * _userInit_stop;
static const struct user_init ** initConfig;


/* USER CODE END Variables */
/* Definitions for defaultTask */


/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */


/* USER CODE END FunctionPrototypes */

void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
	spi1BusInit();
	txComplete = xSemaphoreCreateBinary();
	rxComplete = xSemaphoreCreateBinary();
	spiMutex = xSemaphoreCreateMutex();
	UartRxReady = xSemaphoreCreateBinary();
	CreateUartRxQueue();
	if (txComplete == NULL || rxComplete == NULL || spiMutex == NULL)
	{
	    while (1);
	}
	initConfig = &_userInit_start;
	while(initConfig < &_userInit_stop){
	   (*initConfig)->init();
	   //DEBUG_PRINTF("%s is startup!\n", (*initConfig)->name);
	   initConfig++;
	}

}


