/*
 * dw3000_ll.c
 *
 *  Created on: Aug 4, 2024
 *      Author: twinhorse
 */

#include <string.h>
#include "dwTypes.h"
#include "libdw3000.h"
#include "dw3000.h"
#include "main.h"
#include "dw3000_cbll.h"

/* PHR configuration */
static dwt_config_t uwbPhrConfig = {
    5,            /* Channel number. */
    DWT_PLEN_128, /* Preamble length. Used in TX only. */
    DWT_PAC8,     /* Preamble acquisition chunk size. Used in RX only. */
    9,            /* TX preamble code. Used in TX only. */
    9,            /* RX preamble code. Used in RX only. */
    1,            /* 0 to use standard 8 symbol SFD, 1 to use non-standard 8 symbol, 2 for
                     non-standard 16 symbol SFD and 3 for 4z 8 symbol SDF type */
    DWT_BR_6M8,   /* Data rate. */
#ifdef UWB_ENABLE_PHR_EXT_MODE
    DWT_PHRMODE_EXT, /* Extended PHY header mode. */
#else
    DWT_PHRMODE_STD, /* Standard PHY header mode. */
#endif
    DWT_PHRRATE_STD, /* PHY header rate. */
    (129 + 8 - 8),   /* SFD timeout (preamble length + 1 + SFD length - PAC size).
                        Used in RX only. */
    DWT_STS_MODE_OFF,
    DWT_STS_LEN_64, /* STS length, see allowed values in Enum dwt_sts_lengths_e
                     */
    DWT_PDOA_M0     /* PDOA mode off */
};

static dwt_txconfig_t uwbTxConfigOptions = {
    .PGcount = 0x0,
    .PGdly = 0x34,
    .power = 0xfdfdfdfd
};

#define DW3000_Enable()          LL_GPIO_ResetOutputPin(DW3000_CS_GPIO_Port, DW3000_CS_Pin)
#define DW3000_Disable()         LL_GPIO_SetOutputPin(DW3000_CS_GPIO_Port, DW3000_CS_Pin)

static adhocuwb_hdw_cb_t _txCallback = 0;
static adhocuwb_hdw_cb_t _rxCallback = 0;

/************ Low level ops for libdw **********/

#define SPI_UWB_BUFFER_MAX_SIZE 300
static uint8_t spiUwbTxBuffer[SPI_UWB_BUFFER_MAX_SIZE];
static uint8_t spiUwbRxBuffer[SPI_UWB_BUFFER_MAX_SIZE];

static void spiUwbWrite(const void* cmd,
			size_t cmdLength,
			const void *data,
			size_t dataLength)
{
	spiUwbBeginTransaction();
	DW3000_Enable();
    memcpy(spiUwbTxBuffer, cmd, cmdLength);
    memcpy(spiUwbTxBuffer + cmdLength, data, dataLength);
    spiUwbExchange(cmdLength + dataLength, spiUwbTxBuffer, spiUwbRxBuffer);
	DW3000_Disable();
	spiUwbEndTransaction();
}

static void spiUwbRead(const void* cmd,
			size_t cmdLength,
			void *data,
			size_t dataLength)
{
	spiUwbBeginTransaction();
	DW3000_Enable();
	memcpy(spiUwbTxBuffer, cmd, cmdLength);
	memset(spiUwbTxBuffer + cmdLength, DUMMY_BYTE, dataLength);
	spiUwbExchange(cmdLength + dataLength, spiUwbTxBuffer, spiUwbRxBuffer);
	memcpy(data, spiUwbRxBuffer + cmdLength, dataLength);
	DW3000_Disable();
	spiUwbEndTransaction();
}

static void spiUwbSetSpeed(dwSpiSpeed_t speed) { return; }

static void delayms(unsigned int delay) { vTaskDelay(delay); }

static void reset(void)
{
	LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_3); // Set PC5 low
	vTaskDelay(10);
	LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_3);  // Set PC5 high
	vTaskDelay(10);
}

dwOps_t dwt_ops = {
    .spiRead = spiUwbRead,
    .spiWrite = spiUwbWrite,
    .spiSetSpeed = spiUwbSetSpeed,
    .delayms = delayms,
    .reset = reset
};

/************ Callback functions for libdw **********/
static uint8_t rxBuffer[UWB_FRAME_LEN_MAX];

static void txCallback()
{
  //  DEBUG_PRINT("txCallback \n");
	if(_txCallback)	{
		_txCallback(NULL);
	}
}

static void rxCallback()
{
  uint32_t dataLength = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFLEN_BIT_MASK;

//  ASSERT(dataLength != 0 && dataLength <= UWB_FRAME_LEN_MAX);

  dwt_readrxdata(rxBuffer, dataLength - FCS_LEN, 0); /* No need to read the FCS/CRC. */

  //  DEBUG_PRINT("rxCallback: data length = %lu \n", dataLength);
	if(_rxCallback)	{
		_rxCallback(rxBuffer); //TODO: make this function pass both buffer address and dataLength
	}
  dwt_forcetrxoff();
  dwt_rxenable(DWT_START_RX_IMMEDIATE);
}

static void rxTimeoutCallback()
{
  dwt_forcetrxoff();
  dwt_rxenable(DWT_START_RX_IMMEDIATE);
}

static void rxErrorCallback()
{
  dwt_forcetrxoff();
  dwt_rxenable(DWT_START_RX_IMMEDIATE);
}


int dw3000_init()
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
  dwt_setcallbacks(&txCallback, &rxCallback, &rxTimeoutCallback, &rxErrorCallback, NULL, NULL);

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

void uwbISRTask(void *parameters) {
  vTaskDelay(32);

  while (1)
  {
    if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY))
    {
      do
      {
        xSemaphoreTake(uwbIrqSemaphore, portMAX_DELAY);
        dwt_isr();
        xSemaphoreGive(uwbIrqSemaphore);
      } while (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_13) != RESET);
    }
  }
}

void adhocuwb_hdw_force_rx() {
	dwt_forcetrxoff();
	dwt_rxenable(DWT_START_RX_IMMEDIATE);
}

int adhocuwb_hdw_send(void *data, uint32_t datalen) {
	dwt_forcetrxoff();
	dwt_writetxdata(datalen, (uint8_t *) data, 0);
	dwt_writetxfctrl(datalen + FCS_LEN, 0, 1);
	return (dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED) != DWT_ERROR);
}

void adhocuwb_set_hdw_cbs(adhocuwb_hdw_cb_t txCb, adhocuwb_hdw_cb_t rxCb) {
	_rxCallback = rxCb;
	_txCallback = txCb;
}

