/*
 * spi_sysinit.h
 *
 *  Created on: Jan 23, 2025
 *      Author: mfxjx
 */

#ifndef SYSDRIVER_INC_SPI_SYSINIT_H_
#define SYSDRIVER_INC_SPI_SYSINIT_H_

#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "semphr.h"

void spi1BusInit();

void spi3BusInit();

void spi_sysinit();

#endif /* SYSDRIVER_INC_SPI_SYSINIT_H_ */
