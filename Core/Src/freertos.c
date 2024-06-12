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
#include "uart_hal.h"
#include "tca6408a.h"
#include "vl53l5cx_api.h"
#include "test_tof.h"
#include "calibration.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
static VL53L5CX_Configuration vl53l5dev_f;
static VL53L5CX_ResultsData vl53l5_res_f;
SemaphoreHandle_t txComplete = NULL;
SemaphoreHandle_t rxComplete = NULL;
SemaphoreHandle_t spiMutex = NULL;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define READ_ID_CMD 0x90
#define DUMMY_BYTE 0xFF
#define W25Qx_Enable()          LL_GPIO_ResetOutputPin(W25QXX_CS_GPIO_Port, W25QXX_CS_Pin)
#define W25Qx_Disable()         LL_GPIO_SetOutputPin(W25QXX_CS_GPIO_Port, W25QXX_CS_Pin)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

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

	if (txComplete == NULL || rxComplete == NULL || spiMutex == NULL)
	{
	    // 处理信号量创建失�?
	    while (1);
	}

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
void reset(void){
	W25Qx_Enable();

	spiBeginTransaction(LL_SPI_BAUDRATEPRESCALER_DIV256);

	    // 发�?�复位使能命�?
	    spiExchange(SPI3, 1, 0x66, NULL);

	    // 短暂延迟，确保复位使能命令被处理
	    vTaskDelay(pdMS_TO_TICKS(1));

	    // 发�?�复位命�?
	    spiExchange(SPI3, 1, 0x99, NULL);

	    spiEndTransaction();

	    W25Qx_Disable();

	    // 延迟�?段时间以确保芯片完成复位
	    vTaskDelay(pdMS_TO_TICKS(30));
}
void read_w25q64_id(void)
{
    //reset();
	uint8_t txBuffer[6] = {READ_ID_CMD, 0x00, 0x00, 0x00, 0x00, 0x00}; // 读取ID命令和三个虚拟字�?
    uint8_t rxBuffer[6] = {0};

    W25Qx_Enable();

    spiBeginTransaction(SPI3, LL_SPI_BAUDRATEPRESCALER_DIV256);

    if (spiExchange(SPI3, sizeof(txBuffer), txBuffer, rxBuffer))
    {
        // 读取的ID数据在rxBuffer[1]和rxBuffer[2]�?
        uint8_t manufacturerID = rxBuffer[4];
        uint8_t deviceID = rxBuffer[5];
    }
    else
    {

    }

    spiEndTransaction();
    W25Qx_Disable();
}
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
	  read_w25q64_id();
//
//	  uint8_t ID[4];
//	  I2C_expander_initialize();
//	  initialize_sensors_I2C(&vl53l5dev_f,1);
//	  vl53l5cx_start_ranging(&vl53l5dev_f);
//	  while(1){
//		  LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_9);
//	  	  LL_mDelay(100);
//	  	  get_sensor_data(&vl53l5dev_f, &vl53l5_res_f);
//	  }
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

