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
#include "stm32h7xx_ll_rcc.h"
#include "core_cm7.h"


void WKUP_Init(void)
{
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOC);
    LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_1, LL_GPIO_MODE_INPUT);
    LL_GPIO_SetPinPull(GPIOC, LL_GPIO_PIN_1, LL_GPIO_PULL_UP);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_CEC);
    LL_PWR_EnableWakeUpPin(LL_PWR_WAKEUP_PIN6);
#if (ENABLE_DBG_STANDBY == 1)
    LL_DBGMCU_EnableD1DebugInStopMode();
    LL_DBGMCU_EnableD1DebugInStandbyMode();
#endif
}

void Enter_Stop(void)
{
	LL_PWR_ClearFlag_WU6();
	SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
	LL_PWR_EnableWakeUpPin(LL_PWR_WAKEUP_PIN6);
	LL_PWR_CPU_SetD1PowerMode(LL_PWR_CPU_MODE_D1STOP);
	LL_PWR_CPU_SetD2PowerMode(LL_PWR_CPU_MODE_D2STOP);
	LL_PWR_CPU_SetD3PowerMode(LL_PWR_CPU_MODE_D3STOP);
	__asm volatile( "dsb" ::: "memory" );
	__asm volatile( "wfi" );
	__asm volatile( "isb" );
}


void Enter_Standby(void)
{
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

void Post_Stop(void)
{
	if(LL_PWR_CPU_IsActiveFlag_STOP()!=RESET)
	{
		  LL_RCC_PLL1P_Enable();
		  LL_RCC_PLL1Q_Enable();
		  LL_RCC_PLL1_SetVCOInputRange(LL_RCC_PLLINPUTRANGE_8_16);
		  LL_RCC_PLL1_SetVCOOutputRange(LL_RCC_PLLVCORANGE_MEDIUM);
		  LL_RCC_PLL1_SetM(4);
		  LL_RCC_PLL1_SetN(10);
		  LL_RCC_PLL1_SetP(2);
		  LL_RCC_PLL1_SetQ(2);
		  LL_RCC_PLL1_SetR(2);
		  LL_RCC_PLL1_Enable();
		  while(LL_RCC_PLL1_IsReady() != 1)
		  {
		  }
      LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL1);
	  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL1)
	  {
	  }
      LL_PWR_ClearFlag_CPU();
	  LL_PWR_DisableWakeUpPin(LL_PWR_WAKEUP_PIN6);
	}
}


void standby_example(void)
{
	WKUP_Init();
	while(1)
	{
		LL_mDelay(2000);
		Post_Standby();
		Enter_Standby();
		LL_mDelay(2000);
	}
}


void stop_example(void)
{
	WKUP_Init();
	while(1)
	{
		LL_mDelay(2000);
		Enter_Stop();
		Post_Stop();
		LL_mDelay(2000);
	}
}
