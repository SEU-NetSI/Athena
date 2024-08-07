/*
 * dw3000deck_ll.c
 *
 *  Created on: Aug 4, 2024
 *      Author: twinhorse
 */

#include <string.h>
#include "dw3000deck_ll.h"
#include "dwTypes.h"
#include "libdw3000.h"
#include "dw3000.h"


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


int dw3000Init()
{
  /* Need to make sure DW IC is in IDLE_RC before proceeding */
  for (int i = 0; !dwt_checkidlerc() && i < 3; i++)
  {
  }

  if (!dwt_checkidlerc())
  {
//    DEBUG_PRINT("Error: DW IC is not in IDLE_RC state \n");
    return DWT_ERROR;
  }

  if (dwt_initialise(DWT_DW_INIT) == DWT_ERROR)
  {
    return DWT_ERROR;
  }

  if (dwt_configure(&uwbPhrConfig) == DWT_ERROR)
  {
    return DWT_ERROR;
  }

  dwt_setleds(DWT_LEDS_ENABLE | DWT_LEDS_INIT_BLINK);

  /* Configure the TX spectrum parameters (power, PG delay and PG count) */
  dwt_configuretxrf(&uwbTxConfigOptions);

  dwt_setrxtimeout(0xFFFFF);

  /* Set callback functions */
//  dwt_setcallbacks(&txCallback, &rxCallback, &rxTimeoutCallback, &rxErrorCallback, NULL, NULL);

  /* Enable wanted interrupts (TX confirmation, RX good frames, RX timeouts and RX errors). */
  dwt_setinterrupt(SYS_ENABLE_LO_TXFRS_ENABLE_BIT_MASK |
                       SYS_ENABLE_LO_RXFCG_ENABLE_BIT_MASK |
                       SYS_ENABLE_LO_RXFTO_ENABLE_BIT_MASK |
                       SYS_ENABLE_LO_RXPTO_ENABLE_BIT_MASK |
                       SYS_ENABLE_LO_RXPHE_ENABLE_BIT_MASK |
                       SYS_ENABLE_LO_RXFCE_ENABLE_BIT_MASK |
                       SYS_ENABLE_LO_RXFSL_ENABLE_BIT_MASK |
                       SYS_ENABLE_LO_RXSTO_ENABLE_BIT_MASK,
                   0, DWT_ENABLE_INT);

  /* Clearing the SPI ready interrupt */
  dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RCINIT_BIT_MASK | SYS_STATUS_SPIRDY_BIT_MASK);


  return DWT_SUCCESS;
}
