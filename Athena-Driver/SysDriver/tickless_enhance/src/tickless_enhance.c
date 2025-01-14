/*
 * tickless_enhance.c
 *
 *  Created on: Jan 14, 2025
 *      Author: DELL
 */

#include "tickless_enhance.h"
#include "stm32l4xx_ll_pwr.h"
#include "stm32l4xx_ll_cortex.h"
#include "stm32l4xx_ll_rcc.h"
#include "projdefs.h"
#if(configDEBUG==1)
    #include "stm32l4xx_ll_system.h"
#endif

void vticklessenhanceSLEEP_FOR_TIMER_TASK(uint32_t uLOW_POWER_TIME)
{
	const TickType_t xDelay = pdMS_TO_TICKS(uLOW_POWER_TIME);
	vTaskDelay(xDelay);
}

void vticklessenhancePRE_SLEEP_PROCESSING(uint32_t uLOW_POWER_TIME)
{
    if (uLOW_POWER_TIME<100)
    {
    	LL_LPM_EnableSleep();
        LL_LPM_DisableSleepOnExit();
        #if(configDEBUG==1)
            LL_DBGMCU_EnableDBGSleepMode();
        #endif
    }
    else if (uLOW_POWER_TIME<200)
    {
        LL_LPM_EnableDeepSleep();
        LL_PWR_SetPowerMode(LL_PWR_MODE_STOP0);
        LL_RCC_SetClkAfterWakeFromStop(LL_RCC_STOP_WAKEUPCLOCK_HSI);
        #if(configDEBUG==1)
            LL_DBGMCU_EnableDBGStopMode();
        #endif
    }
    else if (uLOW_POWER_TIME<300)
    {
        LL_LPM_EnableDeepSleep();
        LL_PWR_SetPowerMode(LL_PWR_MODE_STOP1);
        LL_RCC_SetClkAfterWakeFromStop(LL_RCC_STOP_WAKEUPCLOCK_HSI);
        #if(configDEBUG==1)
            LL_DBGMCU_EnableDBGStopMode();
        #endif
	}
    else
    {
        LL_LPM_EnableDeepSleep();
        LL_PWR_SetPowerMode(LL_PWR_MODE_STOP2);
        LL_RCC_SetClkAfterWakeFromStop(LL_RCC_STOP_WAKEUPCLOCK_HSI);
        #if(configDEBUG==1)
            LL_DBGMCU_EnableDBGStopMode();
        #endif
    }
}

void vticklessenhancePOST_SLEEP_PROCESSING(uint32_t uLOW_POWER_TIME)
{
		LL_RCC_HSE_Enable();
		while (LL_RCC_HSE_IsReady() != 1)
		{
		}
		LL_RCC_PLL_Disable();
		LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLLM_DIV_1, 20, LL_RCC_PLLR_DIV_2);
		LL_RCC_PLL_Enable();
		LL_RCC_PLL_EnableDomain_SYS();
		while (LL_RCC_PLL_IsReady() != 1)
		{
		}
		LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
		while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
		{
		}
		LL_Init1msTick(80000000);
}



