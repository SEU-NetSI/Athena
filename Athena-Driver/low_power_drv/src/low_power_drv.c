/*
 * low_power_drv.c
 *
 *  Created on: Jan 16, 2025
 *      Author: Li YunFan
 */
#include "low_power_drv.h"
#include "stm32h7xx_ll_bus.h"
#include "stm32h7xx_ll_gpio.h"
#include "stm32h7xx_ll_pwr.h"
#include "stm32h7xx_ll_system.h"
#include "core_cm7.h"


void WKUP_Init(void)
{
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOC);
    LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_1, LL_GPIO_MODE_INPUT);
    LL_GPIO_SetPinPull(GPIOC, LL_GPIO_PIN_1, LL_GPIO_PULL_UP);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_CEC);
    LL_PWR_EnableWakeUpPin(LL_PWR_WAKEUP_PIN6);
#if (ENABLE_DBG_STANDBY == 1)
    LL_DBGMCU_EnableD1DebugInStandbyMode();
#endif
}

void Enter_Standby(void)
{
	LL_PWR_ClearFlag_WU6();
	SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
	LL_PWR_EnableWakeUpPin(LL_PWR_WAKEUP_PIN6);
	LL_PWR_CPU_SetD1PowerMode(LL_PWR_CPU_MODE_D1STANDBY);
	LL_PWR_CPU_SetD2PowerMode(LL_PWR_CPU_MODE_D2STANDBY);
	LL_PWR_CPU_SetD3PowerMode(LL_PWR_CPU_MODE_D3STANDBY);
	__asm volatile( "dsb" ::: "memory" );
	__asm volatile( "wfi" );
	__asm volatile( "isb" );
}

void Post_Standby(void)
{
	if(LL_PWR_CPU_IsActiveFlag_SB()!=RESET)
	{
      LL_PWR_ClearFlag_CPU();
	  LL_PWR_DisableWakeUpPin(LL_PWR_WAKEUP_PIN6);
	}
}

void standby_example(void)
{
	LL_mDelay(2000);
	Post_Standby();
	WKUP_Init();
	Enter_Standby();
}
