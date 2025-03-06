#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "semphr.h"
#include "main.h"
// 用于初始化USART2的信号量与RXNE中断
SemaphoreHandle_t Uart2RxComplete = NULL;

void USART2_Sysdrv_Init(void)
{
	Uart2RxComplete = xSemaphoreCreateBinary();
	LL_USART_EnableIT_RXNE(USART2);
	printf("USART2 Init Finish!\n");
}

static const UserInit usart2_sysdriver_init = {
		.init = USART2_Sysdrv_Init,
};

USER_INIT(usart2_sysdriver_init);
