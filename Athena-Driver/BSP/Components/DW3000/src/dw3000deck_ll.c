/*
 * dw3000deck_ll.c
 *
 *  Created on: Aug 4, 2024
 *      Author: twinhorse
 */

#include <string.h>
#include "dw3000deck_ll.h"


#define DW3000Deck_Enable()          LL_GPIO_ResetOutputPin(DW3000Deck_CS_GPIO_Port, DW3000Deck_CS_Pin)
#define DW3000Deck_Disable()         LL_GPIO_SetOutputPin(DW3000Deck_CS_GPIO_Port, DW3000Deck_CS_Pin)

/************ Low level ops for libdw **********/

#define SPI_DECK_BUFFER_MAX_SIZE 240
static uint8_t spiDeckTxBuffer[SPI_DECK_BUFFER_MAX_SIZE];
static uint8_t spiDeckRxBuffer[SPI_DECK_BUFFER_MAX_SIZE];

static void spiDeckWrite(const void* cmd,
			size_t cmdLength,
			void *data,
			size_t dataLength)
{
	spiDeckBeginTransaction();
	DW3000Deck_Enable();
    memcpy(spiDeckTxBuffer, cmd, cmdLength);
    memcpy(spiDeckTxBuffer + cmdLength, data, dataLength);
    spiExchange(cmdLength + dataLength, spiDeckTxBuffer, spiDeckRxBuffer);
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

static void delayms(unsigned int delay) { vTaskDelay(M2T(delay)); }

static void reset(void)
{
	LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_5); // Set PC5 low
	LL_mDelay(10);
	LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_5);  // Set PC5 high
	LL_mDelay(10);
}

extern dwOps_t dwt_ops = {
    .spiRead = spiRead,
    .spiWrite = spiWrite,
    .spiSetSpeed = spiSetSpeed,
    .delayms = delayms,
    .reset = reset
};

