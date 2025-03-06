#include "usart_sysinit.h"
#include "debug.h"

fifo_rx_def *pfifo_x = &fifo_usart_rx_2;
SemaphoreHandle_t Uart2RxComplete = NULL;

void USART2_Sysdrv_Init(void)
{
	Uart2RxComplete = xSemaphoreCreateBinary();
	LL_USART_EnableIT_IDLE(USART2);
	LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_6);
	LL_USART_EnableDMAReq_RX(USART2);
	LL_DMA_SetPeriphAddress(
			DMA1,
			LL_DMA_CHANNEL_6,
			LL_USART_DMA_GetRegAddr(USART2, LL_USART_DMA_REG_DATA_RECEIVE)
	);
	DEBUG_PRINTF("USART2 Init Finish!\n");
}

static const UserInit usart2_sysdriver_init = {
		.init = USART2_Sysdrv_Init,
};

USER_INIT(usart2_sysdriver_init);
