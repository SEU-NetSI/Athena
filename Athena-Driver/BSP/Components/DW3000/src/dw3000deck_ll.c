/*
 * dw3000deck_ll.c
 *
 *  Created on: Aug 4, 2024
 *      Author: twinhorse
 */

#include <string.h>
#include "dw3000deck_ll.h"
#include "dwTypes.h"


#define DW3000Deck_Enable()          LL_GPIO_ResetOutputPin(DW3000Deck_CS_GPIO_Port, DW3000Deck_CS_Pin)
#define DW3000Deck_Disable()         LL_GPIO_SetOutputPin(DW3000Deck_CS_GPIO_Port, DW3000Deck_CS_Pin)

/************ Low level ops for libdw **********/

#define SPI_DECK_BUFFER_MAX_SIZE 240
static uint8_t spiDeckTxBuffer[SPI_DECK_BUFFER_MAX_SIZE];
static uint8_t spiDeckRxBuffer[SPI_DECK_BUFFER_MAX_SIZE];

static void spiDeckWrite(const void* cmd,
			size_t cmdLength,
			const void *data,
			size_t dataLength)
{
	spiDeckBeginTransaction();
	DW3000Deck_Enable();
    memcpy(spiDeckTxBuffer, cmd, cmdLength);
    memcpy(spiDeckTxBuffer + cmdLength, data, dataLength);
    spiDeckExchange(cmdLength + dataLength, spiDeckTxBuffer, spiDeckRxBuffer);
	DW3000Deck_Disable();
	spiDeckEndTransaction();
}

static void spiDeckRead(const void* cmd,
			size_t cmdLength,
			void *data,
			size_t dataLength)
{
	spiDeckBeginTransaction();
	DW3000Deck_Enable();
	memcpy(spiDeckTxBuffer, cmd, cmdLength);
	memset(spiDeckTxBuffer + cmdLength, DUMMY_BYTE, dataLength);
	spiDeckExchange(cmdLength + dataLength, spiDeckTxBuffer, spiDeckRxBuffer);
	memcpy(data, spiDeckRxBuffer + cmdLength, dataLength);
	DW3000Deck_Disable();
	spiDeckEndTransaction();
}

//TODO EXTI2_Callback

static void spiDeckSetSpeed(dwSpiSpeed_t speed) { return; }

static void delayms(unsigned int delay) { vTaskDelay(delay); }

static void reset(void)
{
	LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_5); // Set PC5 low
	vTaskDelay(10);
	LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_5);  // Set PC5 high
	vTaskDelay(10);
}

dwOps_t dwt_ops = {
    .spiRead = spiDeckRead,
    .spiWrite = spiDeckWrite,
    .spiSetSpeed = spiDeckSetSpeed,
    .delayms = delayms,
    .reset = reset
};

