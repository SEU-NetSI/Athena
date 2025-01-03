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

#include "DebugPrint_example.h"
#include "../../examples/tinymap/inc/calibration.h"

#include "../../examples/FS_example/src/Flash_FS_Example.c"
//#include "uwb_send_recv_packet_example.c"
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

osThreadId_t ledTaskHandle;
//const osThreadAttr_t ledTask_attributes = {
//  .name = "ledTask",
//  .stack_size = 128 * 2,
//  .priority = (osPriority_t) osPriorityNormal,
//};

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 2,
  .priority = (osPriority_t) osPriorityNormal,
};
osThreadId_t FS_ExampleHandle;
const osThreadAttr_t FS_Example_attributes = {
		.name = "FS_Example",
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
		.stack_size = 128 * 10,
		.priority = (osPriority_t) osPriorityNormal,
};
/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
//static void ledTask(void *argument);
//static void uwbTask(void *argument);

/* USER CODE END FunctionPrototypes */


void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  *
  */

void simpleTxCallback(void *argument) {	// 发送完数据包后的回调函数
	return;
}

void simpleRxCallback(void *argument) {	// 接收到数据包时的回调函数
	uint8_t *packet = (uint8_t *) argument;
	return;
}

osThreadId_t uwbTaskHandle;
const osThreadAttr_t uwbTask_attributes = {
	  .name = "uwbTask",
	  .stack_size = 5 * UWB_FRAME_LEN_MAX * sizeof(StackType_t), //TODO: check whether this works
	  .priority = (osPriority_t) osPriorityNormal,
	};
osThreadId_t uwbISRTaskHandle;

void initUWBConfig(){


	// reset dw3000 chip
	dwt_ops.reset(); // this is not necessary

	// prepare the interrupt service routines task
	uwbISRTaskHandle = osThreadNew(uwbISRTask, NULL, &uwbTask_attributes);
	vTaskDelay(8); // wait for the uwbISRTask to start to handle ISR

	// init the dw3000 chip, get ready to rx and rx，下面两次初始化是为了两个不同SPI配置的dw3000设备
	int result = dw3000_init();
	uint32_t dev_id = dwt_readdevid();
	if (dev_id != 0x0 && dev_id != (0xDECA0302))
	{
	  MX_SPI2_Init_IO2IO3();
	  dw3000_init();
	}
}



void uwbSendRecvPacketTask(void *argument)
{

	initUWBConfig();

	// set the tx and rx callback functions
	adhocuwb_set_hdw_cbs(simpleTxCallback, simpleRxCallback);

	// set the chip in listening mode, rxcallback should be invoked once a packet is received.
	// you should see the RX led flashes at the UWB Deck
	adhocuwb_hdw_force_rx();

	/*============ the above code need only support from BSP/Components/DW3000 =============*/
//	int uwbdata_tx[10] = {1,2,3,4,5,6,7,8,9};
	int uwbdata_tx[2] = {1,2};
	// loop forever
	while(1)
	{
	  adhocuwb_hdw_send(uwbdata_tx, sizeof(uwbdata_tx));
      vTaskDelay(2000);
	}
}

void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
	txComplete = xSemaphoreCreateBinary();
	rxComplete = xSemaphoreCreateBinary();
	spiMutex = xSemaphoreCreateMutex();
	UartRxReady = xSemaphoreCreateBinary();
//	CreateUartRxQueue();
	if (txComplete == NULL || rxComplete == NULL || spiMutex == NULL)
	{
	    while (1);
	}
	spi_deck_init();
	// TOF_exampleHandle = osThreadNew(tof_get_data, NULL, &tof_get_data_attributes);
//	Debug_ExampleHandle = osThreadNew(Debug_example, NULL, &uwb_send_recv_packet_ExampleHandle = osThreadNew(uwbSendRecvPacketTask, NULL, &uwb_send_recv_packet_Example_attributes);Debug_Example_attributes);
//	FS_ExampleHandle = osThreadNew(FS_Example, NULL, &FS_Example_attributes);
//  ledTaskHandle = osThreadNew(ledTask, NULL, &ledTask_attributes);
	uwb_send_recv_packet_ExampleHandle = osThreadNew(uwbSendRecvPacketTask, NULL, &uwb_send_recv_packet_Example_attributes);

}


/* USER CODE END Application */

