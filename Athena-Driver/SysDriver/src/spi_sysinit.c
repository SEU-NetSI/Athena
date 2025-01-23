/*
 * spi_sysinit.c
 *
 *  Created on: Jan 23, 2025
 *      Author: mfxjx
 */

#include "spi_sysinit.h"

SemaphoreHandle_t spi1txComplete = NULL;
SemaphoreHandle_t spi1rxComplete = NULL;
SemaphoreHandle_t spi1Mutex = NULL;
SemaphoreHandle_t txComplete = NULL;
SemaphoreHandle_t rxComplete = NULL;
SemaphoreHandle_t spiMutex = NULL;

void spi1BusInit(){
	spi1txComplete = xSemaphoreCreateBinary();
	spi1rxComplete = xSemaphoreCreateBinary();
	spi1Mutex = xSemaphoreCreateMutex();
	if (spi1txComplete == NULL || spi1rxComplete == NULL || spi1Mutex == NULL)
	{
		Error_Handler();
	}
}

void spi3BusInit(){
	txComplete = xSemaphoreCreateBinary();
	rxComplete = xSemaphoreCreateBinary();
	spiMutex = xSemaphoreCreateMutex();

	if (txComplete == NULL || rxComplete == NULL || spiMutex == NULL)
	{
		while (1);
	}
}

void spi_sysinit(){
	spi1BusInit();
	spi3BusInit();
}

static const UserInit spi_sysDriverInit = {
		.init = spi_sysinit,
};

USER_INIT(spi_sysDriverInit);
