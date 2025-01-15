/*
 * get_neighbor_distances.c
 *
 *  Created on: Jan 15, 2025
 *      Author: twinhorse
 */
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

#include "libdw3000.h"
#include "dw3000.h"
#include "dwTypes.h"
#include "dw3000_cbll.h"
#include "adhocuwb.h"

SemaphoreHandle_t spiDeckTxComplete = NULL;
SemaphoreHandle_t spiDeckRxComplete = NULL;
SemaphoreHandle_t spiDeckMutex = NULL;
SemaphoreHandle_t uwbIrqSemaphore = NULL;

static osThreadId_t uwbISRTaskHandle;

const osThreadAttr_t uwbGetNeighborDistancesTaskAttributes = {
		.name = "uwbGetNeighborDistancesTask",
		.stack_size = 2 * UWB_FRAME_LEN_MAX * sizeof(StackType_t),
		.priority = (osPriority_t) osPriorityNormal,
};

const osThreadAttr_t uwbISRTaskAttributes = {
		.name = "uwbISRTask",
		.stack_size = 2 * UWB_FRAME_LEN_MAX * sizeof(StackType_t),
		.priority = (osPriority_t) osPriorityNormal,
};

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

static void initUWBConfig(){

	// reset dw3000 chip
	dwt_ops.reset(); // this is not necessary

	// prepare the interrupt service routines task
	uwbISRTaskHandle = osThreadNew(uwbISRTask, NULL, &uwbISRTaskAttributes);
	vTaskDelay(100); // wait for the uwbISRTask to start to handle ISR

	// init the dw3000 chip, get ready to rx and rx，下面两次初始化是为了两个不同SPI配置的dw3000设备
	int result = dw3000_init();
	uint32_t dev_id = dwt_readdevid();
	if (dev_id != 0x0 && dev_id != (0xDECA0302))
	{
	  MX_SPI2_Init_IO2IO3();
	  dw3000_init();
	}

	// set the chip in listening mode, rxcallback should be invoked once a packet is received.
	// you should see the RX led flashes at the UWB Deck
	adhocuwb_hdw_force_rx();
}

static void uwbGetNeighborDistancesTask(){
	spi_deck_init();
	initUWBConfig();
	vTaskDelay(100);
	adhocuwbInit();
	while (1) {
		vTaskDelay(1000);
	}

}
osThreadId_t uwbGetNeighborDistancesTaskHandle;

static void uwb_get_distances_init(){
	uwbGetNeighborDistancesTaskHandle = osThreadNew(uwbGetNeighborDistancesTask, NULL, &uwbGetNeighborDistancesTaskAttributes);
}

static const UserInit uwb_distances_init = {
		.init = uwb_get_distances_init,
};

USER_INIT(uwb_distances_init);
