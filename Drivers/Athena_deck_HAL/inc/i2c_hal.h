/*
 * i2c_hal.h
 *
 *  Created on: Apr 10, 2024
 *      Author: mfxjx
 */

#ifndef ATHENA_DECK_HAL_INC_I2C_HAL_H_
#define ATHENA_DECK_HAL_INC_I2C_HAL_H_

#include "stm32l4xx_ll_i2c.h"
#include <stdbool.h>
#include "FreeRTOS.h"
#include "queue.h"

typedef void (*DMA_Callback)(void);

typedef enum
{
  i2cWrite,
  i2cRead
} I2cDirection;

typedef struct _I2cMessage
{
	uint32_t         messageLength;		  //< How many bytes of data to send or received.
	uint8_t          slaveAddress;		  //< The slave address of the device on the I2C bus.
	I2cDirection     direction;         //< Direction of message
	QueueHandle_t    clientQueue;       //< Queue to send received messages to.
	bool             isInternal16bit;   //< Is internal address 16 bit. If false 8 bit.
	uint16_t         internalAddress;   //< Internal address of device.
	uint8_t          *buffer;           //< Pointer to the buffer from where data will be read for transmission, or into which received data will be placed.
} I2cMessage;

#endif /* ATHENA_DECK_HAL_INC_I2C_HAL_H_ */
