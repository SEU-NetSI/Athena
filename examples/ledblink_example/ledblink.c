#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "stm32h7xx_ll_bus.h"

static void ledTask(void *argument);

osThreadId_t ledTaskHandle;

const osThreadAttr_t ledTask_attributes = {
  .name = "ledTask",
  .stack_size = 128 * 2,
  .priority = (osPriority_t) osPriorityNormal,
};

void user_ledtask_init(){
	ledTaskHandle = osThreadNew(ledTask, NULL, &ledTask_attributes);
}

static const UserInit led_init = {
		.init = user_ledtask_init,
};

USER_INIT(led_init);

static void ledTask(void *argument)
{
	  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
	  LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOC);
	  GPIO_InitStruct.Pin = (LL_GPIO_PIN_6 | LL_GPIO_PIN_7);
	  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
	  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
	  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	  LL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	  GPIO_InitStruct.Pin = (LL_GPIO_PIN_10 | LL_GPIO_PIN_11 | LL_GPIO_PIN_12);
	  GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
	  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
	  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  while(1)
  {
	LL_GPIO_TogglePin(GPIOA, LL_GPIO_PIN_12);	//LED1, work correct
	LL_GPIO_TogglePin(GPIOA, LL_GPIO_PIN_11);	//LED2, work correct
	LL_GPIO_TogglePin(GPIOA, LL_GPIO_PIN_10);	//LED3, work correct
	LL_GPIO_TogglePin(GPIOC, LL_GPIO_PIN_7);	//LED4, work correct
	LL_GPIO_TogglePin(GPIOC, LL_GPIO_PIN_6);	//LED5, work correct
	LL_GPIO_TogglePin(GPIOD, LL_GPIO_PIN_13);	//LED6, work correct

	vTaskDelay(500);
  }
}
