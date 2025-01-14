/*
 * tickless_enhance.h
 *
 *  Created on: Jan 14, 2025
 *      Author: DELL
 */

#ifndef TicklessEnhance
#define TicklessEnhance

//#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"

void vticklessenhanceSLEEP_FOR_TIMER_TASK(uint32_t uLOW_POWER_TIME);
void vticklessenhancePRE_SLEEP_PROCESSING(uint32_t uLOW_POWER_TIME);
void vticklessenhancePOST_SLEEP_PROCESSING(uint32_t uLOW_POWER_TIME);

#endif
