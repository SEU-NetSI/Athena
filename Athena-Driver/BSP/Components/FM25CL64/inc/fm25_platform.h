/*
 * platform.h
 *
 *  Created on: Oct 10, 2024
 *      Author: mfxjx
 */

#ifndef BSP_COMPONENTS_FM25CL64_INC_FM25_PLATFORM_H_
#define BSP_COMPONENTS_FM25CL64_INC_FM25_PLATFORM_H_

#include "fm25cl64.h"

#define DUMMY_BYTE 0x00
#define FM25CLXX_CS_GPIO_Port GPIOD
#define FM25CLXX_CS_Pin LL_GPIO_PIN_13

/*读FM25寄存器值*/
void ReadDataFromFM25(uint8_t *rData,uint16_t rSize);

/*写FM25寄存器值*/
void WriteDataToFM25(uint8_t *wData,uint16_t wSize);

/*片选操作*/
void ChipSelectForFM25(FM25CSType cs);

///*写保护操作*/
//void WriteProtectedForFM25(FM25WPType wp);
//
///*保持信号操作*/
//void HoldForFM25(FM25HoldType hold);

#endif /* BSP_COMPONENTS_FM25CL64_INC_FM25_PLATFORM_H_ */
