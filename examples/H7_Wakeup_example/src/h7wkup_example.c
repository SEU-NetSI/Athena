/*
 * h7wkup_example.c
 *
 *  Created on: Jan 15, 2025
 *      Author: Li YunFan
 */
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

#include "h7wkup_example.h"
#include "stm32l4xx_ll_bus.h"
#include "stm32l4xx_ll_gpio.h"

//osThreadId_t wkup_ExampleHandle;
//const osThreadAttr_t wkup_Example_attributes = {
//		.name = "wkup_Example",
//		.stack_size = 128 * 2,
//		.priority = (osPriority_t) osPriorityNormal,
//};


void WKUP_Init(void)
{
    LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = LL_GPIO_PIN_0;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_0);
}

void WKUP_Example(void)
{
	WKUP_Init();
	while(1)
	{
		WKUP_H7();
		LL_mDelay(10000);
	}
}

void WKUP_H7(void)
{
	LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_0);
	LL_mDelay(5);
	LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_0);
}
//void wkup_example_init(){
//	wkup_ExampleHandle = osThreadNew(WKUP_Example, NULL, &wkup_Example_attributes);
//}


//static const UserInit wkup_init = {
//		.init = wkup_example_init,
//};
//
//USER_INIT(wkup_init);
