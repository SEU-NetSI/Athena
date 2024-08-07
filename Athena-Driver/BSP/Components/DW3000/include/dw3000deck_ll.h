/*
 * dw3000deck_ll.h
 *
 *  Created on: Aug 4, 2024
 *      Author: twinhorse
 */

#include "stm32l4xx_ll_gpio.h"
#include "spi_deck.h"
#include "libdw3000.h"


#ifndef BSP_COMPONENTS_DW300_INC_DW3000DECK_LL_H_
#define BSP_COMPONENTS_DW300_INC_DW3000DECK_LL_H_


#define DW3000Deck_CS_GPIO_Port GPIOA
#define DW3000Deck_CS_Pin LL_GPIO_PIN_1

#define DUMMY_BYTE 0x00

extern SemaphoreHandle_t spiDeckTxComplete;
extern SemaphoreHandle_t spiDeckRxComplete;
extern SemaphoreHandle_t spiDeckMutex;

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
//#ifdef UWB_ENABLE_PHR_EXT_MODE
//    DWT_PHRMODE_EXT, /* Extended PHY header mode. */
//#else
    DWT_PHRMODE_STD, /* Standard PHY header mode. */
//#endif
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

int dw3000Init();

#endif /* BSP_COMPONENTS_DW300_INC_DW3000DECK_LL_H_ */
