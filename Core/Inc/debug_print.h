/*
 * debug_print.h
 *
 *  Created on: Mar 4, 2025
 *      Author: Li YunFan
 */

#ifndef INC_DEBUGPRINT_H_
#define INC_DEBUGPRINT_H_

#include "stm32h743xx.h"
#include <stdio.h>

#define DEBUGPRINT_RATE 8000000

void debug_print_init(void)
{
	*(__IO uint32_t*)(0x5C003010) = ((SystemCoreClock / DEBUGPRINT_RATE) - 1); // SWO_CODR
}

int _write(int file, char *ptr, int len)
{
	int DataIdx;

	for (DataIdx = 0; DataIdx < len; DataIdx++)
	{
		ITM_SendChar(*ptr++);
	}
	return len;
}

#endif /* INC_DEBUGPRINT_H_ */
