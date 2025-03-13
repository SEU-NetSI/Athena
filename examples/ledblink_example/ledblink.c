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

int ledcode=0;
	  while(1)
	  {
		  vTaskDelay(100);
		  LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_12);
		  LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_11);
		  LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_10);
		  LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_7);
		  LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_6);
		  LL_GPIO_ResetOutputPin(GPIOD, LL_GPIO_PIN_13);
		  if(ledcode & (0x1UL << 0U))
			  LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_12);
		  if(ledcode & (0x1UL << 1U))
			  LL_GPIO_TogglePin(GPIOA, LL_GPIO_PIN_11);
		  if(ledcode & (0x1UL << 2U))
			  LL_GPIO_TogglePin(GPIOA, LL_GPIO_PIN_10);
		  if(ledcode & (0x1UL << 3U))
			  LL_GPIO_TogglePin(GPIOC, LL_GPIO_PIN_7);
		  if(ledcode & (0x1UL << 4U))
			  LL_GPIO_TogglePin(GPIOC, LL_GPIO_PIN_6);
		  if(ledcode & (0x1UL << 5U))
			  LL_GPIO_TogglePin(GPIOD, LL_GPIO_PIN_13);
		  ledcode++;
  }
}
