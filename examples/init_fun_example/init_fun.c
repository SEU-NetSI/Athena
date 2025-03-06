#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "usart.h"

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

//USER_INIT(led_init);

static void ledTask(void *argument)
{
  uint8_t data[8] = {0xFE,0,0,0,10,0XAA,0xEF};
  while(1)
  {
	Uart2_SendStr(data, 8);
	LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_9);
	vTaskDelay(2000);
  }
}
