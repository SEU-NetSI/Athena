/*
 * w25q64_ll.h
 *
 *  Created on: May 20, 2024
 *      Author: mfxjx
 */

#ifndef BSP_COMPONENTS_W25Q64_INC_W25Q64_LL_H_
#define BSP_COMPONENTS_W25Q64_INC_W25Q64_LL_H_

#include "stm32l4xx_ll_gpio.h"
#include "main.h"


#define READ_ID_CMD 0x90
#define DUMMY_BYTE 0xFF
#define W25Qx_Enable()          LL_GPIO_ResetOutputPin(W25QXX_CS_GPIO_Port, W25QXX_CS_Pin)
#define W25Qx_Disable()         LL_GPIO_SetOutputPin(W25QXX_CS_GPIO_Port, W25QXX_CS_Pin)
void BSP_W25Qx_Read_ID(uint8_t *ID);

#endif /* BSP_COMPONENTS_W25Q64_INC_W25Q64_LL_H_ */
