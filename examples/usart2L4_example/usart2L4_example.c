#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "usart.h"
#include "stm32h7xx_ll_usart.h"

static void usart2L4Task(void *argument);

osThreadId_t usart2L4TaskHandle;
extern SemaphoreHandle_t Uart2RxComplete;

const osThreadAttr_t usart2L4Task_attributes = {
  .name = "usart2L4Task",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};


void user_usart2L4Task_init(){
	usart2L4TaskHandle = osThreadNew(usart2L4Task, NULL, &usart2L4Task_attributes);
	printf("usart2L4Task init ok!\n");
}

static const UserInit usart2L4_init = {
		.init = user_usart2L4Task_init,
};

USER_INIT(usart2L4_init);

// H7中用于响应L4指令的后台任务
static void usart2L4Task(void *argument)
{
//	测试发送
	uint8_t data[16];
	for(int i =0; i < 15;++i){
		data[i] = i;// 16字节数据
	}
//    for(;;)
//    {
//    	Uart2_SendStr(data,16);
//        LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_9);
//        osDelay(2000);
//    }
	printf("usart2L4Task Start\n");
	for(;;)
	{
		if(xSemaphoreTake(Uart2RxComplete, pdMS_TO_TICKS(1000)) == pdPASS)
		{

			Uart2_SendStr(data,16);
			osDelay(1);
		}
//		LL_USART_TransmitData8(USART2, 0xBB);
//		LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_9);
//		osDelay(1000);
	}
}

