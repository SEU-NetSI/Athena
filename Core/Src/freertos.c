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
#include "uart_hal.h"
#include "tca6408a.h"
#include "vl53l5cx_api.h"
#include "w25q64_ll.h"
#include "uart_receive.h"
#include "libdw3000.h"
#include "dw3000.h"
#include "dwTypes.h"
#include "dw3000_cbll.h"
#include "tof_get_data.c"
#include "usart.h"

#include "DebugPrint_example.h"
#include "../../examples/tinymap/inc/calibration.h"

#include "../../examples/FS_example/src/Flash_FS_Example.c"
#include "uwb_send_recv_packet_example.c"
#include "cpx/cpx.h"
#include "cpx/uart_transport.h"
#include "cpx/router.h"
#include "../../examples/cpx_example/cpx_example.c"
//#include "adhocuwb.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
SemaphoreHandle_t txComplete = NULL;
SemaphoreHandle_t rxComplete = NULL;
SemaphoreHandle_t spiMutex = NULL;
SemaphoreHandle_t UartRxReady = NULL;
SemaphoreHandle_t spiDeckTxComplete = NULL;
SemaphoreHandle_t spiDeckRxComplete = NULL;
SemaphoreHandle_t spiDeckMutex = NULL;
SemaphoreHandle_t uwbIrqSemaphore = NULL;
uint8_t uwbdata_tx[260];

extern const struct user_init * _userInit_start;
extern const struct user_init * _userInit_stop;
static const struct user_init ** initConfig;





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

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 2,
  .priority = (osPriority_t) osPriorityNormal,
};
osThreadId_t FS_WriteHandle;
const osThreadAttr_t FS_Write_attributes = {
		.name = "Data_Write",
		.stack_size = 2000,
		.priority = (osPriority_t) osPriorityNormal,
};
osThreadId_t FS_ReadHandle;
const osThreadAttr_t FS_Read_attributes = {
		.name = "Data_Read",
		.stack_size = 128 * 4,
		.priority = (osPriority_t) osPriorityNormal,
};
osThreadId_t TOF_exampleHandle;
const osThreadAttr_t tof_get_data_attributes = {
		.name = "tof_get_data",
		.stack_size = 128 * 4,
		.priority = (osPriority_t) osPriorityNormal,
};
osThreadId_t Debug_ExampleHandle;
const osThreadAttr_t Debug_Example_attributes = {
		.name = "Debug_example",
		.stack_size = 128 * 2,
		.priority = (osPriority_t) osPriorityNormal,
};

osThreadId_t uwb_send_recv_packet_ExampleHandle;
const osThreadAttr_t uwb_send_recv_packet_Example_attributes = {
		.name = "uwbSendRecvPacketTask",
		.stack_size = 2 * UWB_FRAME_LEN_MAX * sizeof(StackType_t),
		.priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */


/* USER CODE END FunctionPrototypes */




void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
	txComplete = xSemaphoreCreateBinary();
	rxComplete = xSemaphoreCreateBinary();
	spiMutex = xSemaphoreCreateMutex();
	UartRxReady = xSemaphoreCreateBinary();
	CreateUartRxQueue();
	if (txComplete == NULL || rxComplete == NULL || spiMutex == NULL)
	{
	    while (1);
	}
	spi_deck_init();
  
	// TOF_exampleHandle = osThreadNew(tof_get_data, NULL, &tof_get_data_attributes);
//	Debug_ExampleHandle = osThreadNew(Debug_example, NULL, &uwb_send_recv_packet_ExampleHandle = osThreadNew(uwbSendRecvPacketTask, NULL, &uwb_send_recv_packet_Example_attributes);Debug_Example_attributes);
//	FS_ExampleHandle = osThreadNew(FS_Example, NULL, &FS_Example_attributes);
//  ledTaskHandle = osThreadNew(ledTask, NULL, &ledTask_attributes);
	// uwb_send_recv_packet_ExampleHandle = osThreadNew(uwbSendRecvPacketTask, NULL, &uwb_send_recv_packet_Example_attributes);

	initConfig = &_userInit_start;
	while(initConfig < &_userInit_stop){
	   (*initConfig)->init();
	   initConfig++;
	}


// TOF_exampleHandle = osThreadNew(tof_get_data, NULL, &tof_get_data_attributes);
//	Debug_ExampleHandle = osThreadNew(Debug_example, NULL, &Debug_Example_attributes);
//	FS_ExampleHandle = osThreadNew(Data_Init, NULL, &FS_Example_attributes);
//	FS_WriteHandle = osThreadNew(Data_Write, NULL, &FS_Write_attributes);
//	FS_ReadHandle = osThreadNew(Data_Read, NULL, &FS_Read_attributes);
//    ledTaskHandle = osThreadNew(ledTask, NULL, &ledTask_attributes);
//  uwbTaskHandle = osThreadNew(uwbTask, NULL, &uwbTask_attributes);
}


