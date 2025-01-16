/*
 * lptim_tickless.c
 *
 *  Created on: Jan 14, 2025
 *      Author: Li YunFan
 */

#include "FreeRTOS.h"
#include "task.h"
#include "stm32l4xx.h"
#include "stm32l4xx_ll_bus.h"
#include "stm32l4xx_ll_rcc.h"
#include "stm32l4xx_ll_system.h"
#include "stm32l4xx_ll_cortex.h"


#if ( !defined(configUSE_TICKLESS_IDLE) || configUSE_TICKLESS_IDLE != 2 )
#warning Please edit FreeRTOSConfig.h to define configUSE_TICKLESS_IDLE as 2 *or* exclude this file.
#else

#ifdef xPortSysTickHandler
#warning Please edit FreeRTOSConfig.h to eliminate the preprocessor definition for xPortSysTickHandler.
#endif

#ifndef configTICK_INTERRUPT_PRIORITY
#define configTICK_INTERRUPT_PRIORITY configLIBRARY_LOWEST_INTERRUPT_PRIORITY
#endif

#ifdef configTICK_USES_LSI
   #define LPTIMSEL_Val 1
   #define IS_REF_CLOCK_READY() (RCC->CSR & RCC_CSR_LSIRDY)
#else
   #define LPTIMSEL_Val 3
   #define IS_REF_CLOCK_READY() (RCC->BDCR & RCC_BDCR_LSERDY)
#endif

#define ticklessTICK_RATE_HZ     100

#ifndef configLPTIM_REF_CLOCK_HZ
#define configLPTIM_REF_CLOCK_HZ 32768UL
#endif

#ifndef configLPTIM_ENABLE_PRECISION
#define configLPTIM_ENABLE_PRECISION 1
#endif

#ifndef traceTICKS_DROPPED
#define traceTICKS_DROPPED(x)
#endif

#define LPTIM_CLOCK_HZ ( configLPTIM_REF_CLOCK_HZ )

static TickType_t xMaximumSuppressedTicks;
static uint32_t ulTimerCountsForOneTick;
#if ( configLPTIM_ENABLE_PRECISION != 0 )

   static int lSubcountErrorPerTick;
   static volatile int lRunningSubcountError;

#endif

static volatile uint16_t usIdealCmp;
static volatile uint8_t isCmpWriteInProgress;
static volatile uint8_t isTickNowSuppressed;

#ifndef LPTIM
#define LPTIM              LPTIM1
#define LPTIM_IRQn         LPTIM1_IRQn
#define LPTIM_IRQHandler   LPTIM1_IRQHandler
#endif

void vPortSetupTimerInterrupt( void )
{
   LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_LPTIM1);
   LL_RCC_LSI_Enable();
   while(LL_RCC_LSI_IsReady() != 1);
   LL_RCC_SetLPTIMClockSource(LL_RCC_LPTIM1_CLKSOURCE_LSI);
   #if(configDEBUG==1)
      LL_DBGMCU_APB1_GRP1_FreezePeriph(LL_DBGMCU_APB1_GRP1_LPTIM1_STOP);
   #endif
   LL_APB1_GRP1_ForceReset(LL_APB1_GRP1_PERIPH_LPTIM1);
   LL_APB1_GRP1_ReleaseReset(LL_APB1_GRP1_PERIPH_LPTIM1);

   configASSERT(IS_REF_CLOCK_READY());

   ulTimerCountsForOneTick = ( LPTIM_CLOCK_HZ + ( configTICK_RATE_HZ / 2 ) ) / configTICK_RATE_HZ;
   configASSERT( ulTimerCountsForOneTick >= 4UL );

   #if ( configLPTIM_ENABLE_PRECISION == 0 )
   {
      xMaximumSuppressedTicks = 65536UL / ulTimerCountsForOneTick - 1 - 1;
   }
   #else
   {
      xMaximumSuppressedTicks = 65536UL * configTICK_RATE_HZ / LPTIM_CLOCK_HZ - 1 - 1;

      #if ( LPTIM_CLOCK_HZ % ticklessTICK_RATE_HZ < ticklessTICK_RATE_HZ/2 )
         #define IS_SUBCOUNT_EPT_POSITIVE 1
      #else
         #define IS_SUBCOUNT_EPT_POSITIVE 0
      #endif

      lSubcountErrorPerTick = LPTIM_CLOCK_HZ - ( ulTimerCountsForOneTick * configTICK_RATE_HZ );
      configASSERT( lSubcountErrorPerTick != configTICK_RATE_HZ / 2 );
   }
   #endif

   LPTIM->IER = LPTIM_IER_CMPMIE | LPTIM_IER_CMPOKIE;
   LPTIM->CFGR = (1 << LPTIM_CFGR_PRESC_Pos);
   LPTIM->CR = LPTIM_CR_ENABLE;
   LPTIM->ARR = 0xFFFF;
   LPTIM->CMP = ulTimerCountsForOneTick;
   isCmpWriteInProgress = pdTRUE;
   usIdealCmp = ulTimerCountsForOneTick;
   #if ( configLPTIM_ENABLE_PRECISION != 0 )
   {
      lRunningSubcountError = lSubcountErrorPerTick;
   }
   #endif
   LPTIM->CR |= LPTIM_CR_CNTSTRT;

   NVIC_SetPriority( LPTIM_IRQn, configTICK_INTERRUPT_PRIORITY );
   NVIC_EnableIRQ( LPTIM_IRQn );

   /* SysTick config, only on Stm32L4 */
   SysTick->CTRL = 0UL;
   SysTick->VAL = 0UL;
   SysTick->LOAD = (configCPU_CLOCK_HZ / configTICK_RATE_HZ) - 1UL;
   SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
}


void vPortSuppressTicksAndSleep( TickType_t xExpectedIdleTime )
{
   if (xExpectedIdleTime > xMaximumSuppressedTicks)
   {
      xExpectedIdleTime = xMaximumSuppressedTicks;
   }

   #if ( configLPTIM_ENABLE_PRECISION != 0 )
      uint32_t ulExtraCounts = (xExpectedIdleTime - 1UL) * LPTIM_CLOCK_HZ / configTICK_RATE_HZ;
      int32_t lExtraError = (xExpectedIdleTime - 1UL) * LPTIM_CLOCK_HZ % configTICK_RATE_HZ;
   #else
      uint32_t ulExtraCounts = (xExpectedIdleTime - 1UL) * ulTimerCountsForOneTick;
   #endif

   LL_SYSTICK_DisableIT();
   __disable_irq();

   if (eTaskConfirmSleepModeStatus() == eAbortSleep)
   {
      __enable_irq();
   }
   else
   {
      #if ( configLPTIM_ENABLE_PRECISION != 0 )
      {
         lRunningSubcountError += lExtraError;
         if (lRunningSubcountError > (int)(configTICK_RATE_HZ/2))
         {
            ulExtraCounts++;
            lRunningSubcountError -= configTICK_RATE_HZ;
         }
      }
      #endif

      isTickNowSuppressed = pdTRUE;

      usIdealCmp += ulExtraCounts;
      if (!isCmpWriteInProgress)
      {
         isCmpWriteInProgress = pdTRUE;
         LPTIM->CMP = usIdealCmp == 0xFFFF ? 0 : usIdealCmp;
      }
      uint32_t ulExpectedEndCmp = usIdealCmp;

      do
      {
         TickType_t xModifiableIdleTime = xExpectedIdleTime;
         configPRE_SLEEP_PROCESSING( xModifiableIdleTime );
         if (xModifiableIdleTime > 0)
         {
            __DSB();
            __WFI();
         }
         configPOST_SLEEP_PROCESSING( (const TickType_t)xExpectedIdleTime );

         __enable_irq();
         __ISB();

         __disable_irq();

      } while (usIdealCmp == ulExpectedEndCmp && eTaskConfirmSleepModeStatus() != eAbortSleep);

      __enable_irq();

      TickType_t xCompleteTickPeriods = xExpectedIdleTime - (TickType_t)1;

      portDISABLE_INTERRUPTS();
      if (usIdealCmp == ulExpectedEndCmp)
      {
         uint32_t ulCurrCount;
         do ulCurrCount = LPTIM->CNT; while (ulCurrCount != LPTIM->CNT);

         uint32_t ulFullCountsLeft = (uint16_t)(usIdealCmp - ulCurrCount - 1UL);
         #if ( configLPTIM_ENABLE_PRECISION == 0 )
         if (ulFullCountsLeft < xMaximumSuppressedTicks * ulTimerCountsForOneTick)
         #else
         if (ulFullCountsLeft <= xMaximumSuppressedTicks * LPTIM_CLOCK_HZ / configTICK_RATE_HZ)
         #endif
         {
            TickType_t xFullTicksLeft;
            #if ( configLPTIM_ENABLE_PRECISION != 0 )
            {
               xFullTicksLeft = ulFullCountsLeft * configTICK_RATE_HZ / LPTIM_CLOCK_HZ;
               if (xFullTicksLeft == xExpectedIdleTime)
               {
                  xFullTicksLeft = xExpectedIdleTime - (TickType_t)1;
               }
            }
            #else
            {
               xFullTicksLeft = ulFullCountsLeft / ulTimerCountsForOneTick;
            }
            #endif
            configASSERT( xFullTicksLeft < xExpectedIdleTime );

            if (xFullTicksLeft != 0)
            {
               xCompleteTickPeriods -= xFullTicksLeft;

               uint32_t ulFullTicksLeftAsCounts;
               #if ( configLPTIM_ENABLE_PRECISION != 0 )
               {
                  ulFullTicksLeftAsCounts = xFullTicksLeft * LPTIM_CLOCK_HZ / configTICK_RATE_HZ;
                  lExtraError = xFullTicksLeft * LPTIM_CLOCK_HZ % configTICK_RATE_HZ;
                  lRunningSubcountError -= lExtraError;
                  if (lRunningSubcountError < -(int)(configTICK_RATE_HZ/2))
                  {
                     ulFullTicksLeftAsCounts++;
                     lRunningSubcountError += configTICK_RATE_HZ;
                  }
               }
               #else
               {
                  ulFullTicksLeftAsCounts = xFullTicksLeft * ulTimerCountsForOneTick;
               }
               #endif

               usIdealCmp -= ulFullTicksLeftAsCounts;
               if (!isCmpWriteInProgress)
               {
                  isCmpWriteInProgress = pdTRUE;
                  LPTIM->CMP = usIdealCmp == 0xFFFF ? 0 : usIdealCmp;
               }
            }
         }
      }

      isTickNowSuppressed = pdFALSE;
      portENABLE_INTERRUPTS();
      LL_SYSTICK_EnableIT();

      vTaskStepTick( xCompleteTickPeriods );
   }
}


void LPTIM_IRQHandler( void )
{
   if (LPTIM->ISR & LPTIM_ISR_CMPM)
   {
      LPTIM->ICR = LPTIM_ICR_CMPMCF;
   }

   uint32_t ulCountValue;
   do ulCountValue = LPTIM->CNT; while (ulCountValue != LPTIM->CNT);
   uint32_t ulCountsLate = (uint16_t)(ulCountValue - usIdealCmp);

   if (ulCountsLate >= ulTimerCountsForOneTick &&
       ulCountsLate < 65536UL - 1 - ulTimerCountsForOneTick &&
       !isTickNowSuppressed)
   {
      traceTICKS_DROPPED( ulCountsLate / ulTimerCountsForOneTick );
      usIdealCmp = ulCountValue;
      ulCountsLate = 0;
   }

   if (ulCountsLate < ulTimerCountsForOneTick)
   {
      uint32_t ulNumCounts = ulTimerCountsForOneTick;
      #if ( configLPTIM_ENABLE_PRECISION != 0 )
      {
         lRunningSubcountError += lSubcountErrorPerTick;
         #if ( IS_SUBCOUNT_EPT_POSITIVE )
         {
            if (lRunningSubcountError >= (int)(configTICK_RATE_HZ/2))
            {
               ulNumCounts++;
               lRunningSubcountError -= configTICK_RATE_HZ;
            }
         }
         #else
         {
            if (lRunningSubcountError <= -(int)(configTICK_RATE_HZ/2))
            {
               ulNumCounts--;
               lRunningSubcountError += configTICK_RATE_HZ;
            }
         }
         #endif
      }
      #endif

      usIdealCmp += ulNumCounts;
      if (!isCmpWriteInProgress)
      {
         LPTIM->CMP = usIdealCmp == 0xFFFF ? 0 : usIdealCmp;
         isCmpWriteInProgress = pdTRUE;
      }

      portDISABLE_INTERRUPTS();
      BaseType_t xWasHigherPriorityTaskWoken = xTaskIncrementTick();
      portENABLE_INTERRUPTS();

      portYIELD_FROM_ISR(xWasHigherPriorityTaskWoken);
   }

   if (LPTIM->ISR & LPTIM_ISR_CMPOK)
   {
      LPTIM->ICR = LPTIM_ICR_CMPOKCF;
      if ((uint16_t)(LPTIM->CMP - usIdealCmp) > 1UL)
      {
         LPTIM->CMP = usIdealCmp == 0xFFFF ? 0 : usIdealCmp;
      }
      else
      {
         isCmpWriteInProgress = pdFALSE;
      }
   }
}

#endif

