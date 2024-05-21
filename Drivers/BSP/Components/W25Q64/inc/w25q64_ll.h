/*
 * w25q64_ll.h
 *
 *  Created on: May 20, 2024
 *      Author: mfxjx
 */

#ifndef BSP_COMPONENTS_W25Q64_INC_W25Q64_LL_H_
#define BSP_COMPONENTS_W25Q64_INC_W25Q64_LL_H_

#include "stm32l4xx_ll_gpio.h"

#define W25Qx_Enable() 			LL_GPIO_ResetOutputPin(GPIOB,LL_GPIO_PIN_12)
#define W25Qx_Disable() 		LL_GPIO_SetOutputPin(GPIOB,LL_GPIO_PIN_12)
#define RESET_ENABLE_CMD		0x66
#define RESET_MEMORY_CMD		0x99
#define READ_STATUS_REG1_CMD    0x05
#define W25Q128FV_FSR_BUSY      ((uint8_t)0x01)    /*!< busy */
#define W25Qx_BUSY          ((uint8_t)0x02)
#define W25Qx_OK            ((uint8_t)0x00)
#define READ_ID_CMD         0x90

uint8_t LL_BSP_W25Qx_Init(void);
static void	LL_BSP_W25Qx_Reset(void);
static uint8_t LL_BSP_W25Qx_GetStatus(void);
void LL_BSP_W25Qx_Read_ID(uint8_t *ID);


#endif /* BSP_COMPONENTS_W25Q64_INC_W25Q64_LL_H_ */
