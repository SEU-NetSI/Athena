#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

#include "cpx/cpx.h"
#include "cpx/uart_transport.h"
#include "cpx/router.h"

static CPXPacket_t cpxPacket;

static void cpxPacketCallback(const CPXPacket_t *cpxRx)
{
  DEBUG_PRINTF("Got packet from cf (%u)\n", cpxRx->data[0]);
}

static void cpx_Example(void *argument)
{
	DEBUG_PRINTF("led task is up\n");
	cpxRegisterAppMessageHandler(cpxPacketCallback);
	DEBUG_PRINTF("to call uart_transport_init\n");
	uart_transport_init();
	routerInit();
	DEBUG_PRINTF("router initialized\n");
	cpxInit();
	uint8_t count=0;
  while(1)
  {
	    vTaskDelay(2000);
	    cpxInitRoute(CPX_T_GAP8, CPX_T_STM32, CPX_F_APP, &cpxPacket.route);
	    cpxPacket.dataLength = 1;
	    cpxPacket.data[0]=count;
	    count++;
	    cpxSendPacketBlocking(&cpxPacket);
	    DEBUG_PRINTF("send packet to cf(%u).\n",cpxPacket.data[0]);
  }
}
