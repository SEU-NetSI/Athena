/*
 * low_power_drv.h
 *
 *  Created on: Jan 16, 2025
 *      Author: Li YunFan
 */

#ifndef LOW_POWER_DRV
#define LOW_POWER_DRV

#ifndef ENABLE_DBG_STANDBY
#define ENABLE_DBG_STANDBY 0
#endif

void WKUP_Init(void);
void Enter_Standby(void);
void Post_Standby(void);
void standby_example(void);

#endif
