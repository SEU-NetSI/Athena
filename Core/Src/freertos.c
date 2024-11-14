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
#include "test_tof.h"
#include "calibration.h"
#include "w25q64_ll.h"
#include "uart_receive.h"
#include "libdw3000.h"
#include "dw3000.h"
#include "dwTypes.h"
#include "dw3000_cbll.h"
#include "adhocuwb.h"


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
static uint8_t Pos[26];
static uint8_t Pos_new[17];
int all_pos[3][3];
float para[4];
static float padX = 0.0;
static float padY = 0.0;
static float padZ = 0.0;
static bool flag = 0;
//拔尖基地展示
float datas_f[6];

//拔尖基地展示
void initData(){
        for(int i=0;i<6;i++){
                datas_f[i]=1.0f;
        }
}
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

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
osThreadId_t ledTaskHandle;
const osThreadAttr_t ledTask_attributes = {
  .name = "ledTask",
  .stack_size = 128 * 2,
  .priority = (osPriority_t) osPriorityNormal,
};
osThreadId_t uwbTaskHandle;
const osThreadAttr_t uwbTask_attributes = {
  .name = "uwbTask",
  .stack_size = 2 * UWB_FRAME_LEN_MAX * sizeof(StackType_t), //TODO: check whether this works
  .priority = (osPriority_t) osPriorityNormal,
};
osThreadId_t uwbISRTaskHandle;
/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
static void ledTask(void *argument);
static void uwbTask(void *argument);

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
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

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  ledTaskHandle = osThreadNew(ledTask, NULL, &ledTask_attributes);
  uwbTaskHandle = osThreadNew(uwbTask, NULL, &uwbTask_attributes);
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
void para_get()
{
  padX = para[0];
  padY = para[1];
  padZ = para[2];
}

void para_reget()
{
  para[0] = padX;
  para[1] = padY;
  para[2] = padZ;
}

void compute()
{
	switch(Pos[24])
	{
	case 0:
		//add more control 1up 0down
		if(Pos[25])
		{
			padZ = 0.3f;
			Pos_new[16] = 1;
		}
		else
		{
			Pos_new[16] = 0;
		}
		break;
	case 1:
		if(Pos[25])
		{
			if(padY > 0.8f)
			{
				flag = 1;
			}
			if(flag == 0)
			{
				padY += 0.1f;
			}
			else
			{
				padY -= 0.1f;
			}
			Pos_new[16] = 2;
			if(padY < -1.2f && flag == 1)
			{
				padZ = 0.3f;
				Pos_new[16] = 3;
			}
		}
		else
		{
			padZ = 0.3f;
			Pos_new[16] = 3;
		}
		break;
	}
}
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
    uint8_t index = 0;
	  while(1)
	  {
      if (xSemaphoreTake(UartRxReady, 0) == pdPASS)
      {
        while (index < 26 && xQueueReceive(UartRxQueue, &Pos[index], 0) == pdPASS) 
        {
				  index++;
        }
        if(index == 26)
		    {
          memcpy(para, (float *)Pos, 16);
          para_get();
          compute();
          para_reget();
          memcpy(Pos_new, (uint8_t *)para, 16);
		      UART_DMA_Transmit(Pos_new, 17);
		      index=0;
		    }
      }
		  vTaskDelay(1);
	  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

void simple_txCallback(void *argument) {
	return;
}

void simple_rxCallback(void *argument) {
	uint8_t *packet = (uint8_t *) argument;
	return;
}

static void uwbTask(void *argument)
{
	int result = 0;
	led_flash_in_rpm = 750;

	// reset dw3000 chip
	dwt_ops.reset(); // this is not necessary

	// prepare the interrupt service routines task
	uwbISRTaskHandle = osThreadNew(uwbISRTask, NULL, &uwbTask_attributes);
	vTaskDelay(8); // wait for the uwbISRTask to start to handle ISR

	// init the dw3000 chip, get ready to rx and rx
	result = dw3000_init();
	uint32_t  dev_id;
	dev_id = dwt_readdevid();
	if (dev_id != 0x0 && dev_id != (0xDECA0302))
	{
	  MX_SPI2_Init_IO2IO3();
	  result = dw3000_init();
	}

	// set the tx and rx callback functions
//	adhocuwb_set_hdw_cbs(simple_txCallback, simple_rxCallback);

	// set the chip in listening mode, rxcallback should be invoked once a packet is received.
	// you should see the RX led flashes at the UWB Deck
	adhocuwb_hdw_force_rx();

	vTaskDelay(1000);

	// transmit data with length, txcallback should be invoked once tx success
	// you should see the TX led flashes at the UWB Deck
//	uint8_t uwbdata_tx[32] = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC};

	for(int i=0;i<259;i++){
		uwbdata_tx[i] = i;
	}

//	result = adhocuwb_hdw_send(uwbdata_tx, 240);

	/*============ the above code need only support from BSP/Components/DW3000 =============*/

	vTaskDelay(1000);

	/*============  the following code need additional support from AdHocUWB   =============*/
	adhocuwbInit();

	// loop forever
	while(1)
	{
//		result = adhocuwb_hdw_send(uwbdata_tx, 30);
//		result = adhocuwb_hdw_send(uwbdata_tx, 250);
      vTaskDelay(2000);
	}
}

static void ledTask(void *argument)
{
  while(1)
  {
	LL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
	vTaskDelay(30000 / led_flash_in_rpm);
  }
}
/* USER CODE END Application */

