/*
 * i2c_hal.c
 *
 *  Created on: Apr 10, 2024
 *      Author: mfxjx
 */

#include "stm32l4xx.h"
#include "stm32l4xx_ll_i2c.h"
#include "i2c_hal.h"
#include "stm32l4xx_ll_dma.h"
#include "retarget.h"
#include "semphr.h"

#include <stdio.h>
#include <stdbool.h>

SemaphoreHandle_t isBusFreeSemaphore; //< Semaphore to block during transaction.
StaticSemaphore_t isBusFreeSemaphoreBuffer;
SemaphoreHandle_t isBusFreeMutex;     //< Mutex to protect bus
StaticSemaphore_t isBusFreeMutexBuffer;

bool i2cstartTransfer(){

}
//I2C Transfer Function
/*
 * 1. 判断发来的数据中是读还是写，不管是读，还是写，第一步都是写。
 * 2. 判断发来的数据内部寄存器是8bit还是16bit，这只和第一步的写有关系
 * 3. 分化，如果是写入，使用DMA方式（待开发），写入
 * 4. 如果是读出，使用DMA方式，读出
 * */

bool i2cTransferMessage(I2cMessage *message)
{
	bool status = false;

	xSemaphoreTake(i2c->isBusFreeMutex, portMAX_DELAY);

	i2cstartTransfer(message);





}
