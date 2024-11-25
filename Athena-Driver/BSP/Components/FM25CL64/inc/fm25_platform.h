/*
 * fm25_platform.h
 *
 *  Created on: Nov 23, 2024
 *      Author: Hanjie Yu
 */

#ifndef BSP_COMPONENTS_FM25CL64_INC_FM25_PLATFORM_H_
#define BSP_COMPONENTS_FM25CL64_INC_FM25_PLATFORM_H_

#include "fm25cl64.h"

#define DUMMY_BYTE 0x00

/*读FM25寄存器值*/
void ReadDataFromFM25(uint8_t *rData,uint16_t rSize);

/*写FM25寄存器值*/
void WriteDataToFM25(uint8_t *wData,uint16_t wSize);

/*片选操作*/
void ChipSelectForFM25(FM25CSType cs);

#endif /* BSP_COMPONENTS_FM25CL64_INC_FM25_PLATFORM_H_ */
