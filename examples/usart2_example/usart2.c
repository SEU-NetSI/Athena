#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "usart_sysinit.h"
#include "debug.h"

extern SemaphoreHandle_t Uart2RxComplete;
extern fifo_rx_def *pfifo_x;

static void usart2Task(void* argument);

osThreadId_t usart2TaskHandle;

const osThreadAttr_t usart2Task_attributes = {
  .name = "usart2Task",
  .stack_size = 128 * 2,
  .priority = (osPriority_t) osPriorityNormal,
};

void user_usart2task_init(){
	usart2TaskHandle = osThreadNew(usart2Task, NULL, &usart2Task_attributes);
	DEBUG_PRINTF("USART2 Task Start\n");
}

static const UserInit usart2_init = {
		.init = user_usart2task_init,
};

//USER_INIT(usart2_init);

// L4中用于接收H7返回数据的后台任务
static void usart2Task(void* argument)
{
	for(;;)
	  {
			if(xSemaphoreTake(Uart2RxComplete, pdMS_TO_TICKS(1000)) == pdPASS)
		    {
		    uint8_t ucRawData[64] = {0};
		    uint16_t usLen = fifo_read_buff(pfifo_x, ucRawData, sizeof(ucRawData));
		    osDelay(1);
		    }
	  }
}
