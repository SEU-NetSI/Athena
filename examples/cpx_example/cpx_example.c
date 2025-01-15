#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

#include "cpx/cpx.h"
#include "cpx/uart_transport.h"
#include "cpx/router.h"

osThreadId_t cpx_ExampleHandle;
const osThreadAttr_t cpx_Example_attributes = {
		.name = "cpx_Example",
		.stack_size = 128 * 2,
		.priority = (osPriority_t) osPriorityNormal,
};

static CPXPacket_t cpxPacket;

static void cpxPacketCallback(const CPXPacket_t *cpxRx)
{
	DEBUG_PRINTF("Got Data from STM32: [");
	        for (size_t i = 0; i < cpxRx->dataLength; i++)
	        {
	            DEBUG_PRINTF("%d\n", cpxRx->data[i]);
	        }
	        DEBUG_PRINTF("]\n");
}

static void cpx_Example(void *argument)
{
	DEBUG_PRINTF("cpx example task is up\n");
	cpxRegisterAppMessageHandler(cpxPacketCallback);
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
	    //DEBUG_PRINTF("send packet to cf(%u).\n",cpxPacket.data[0]);
  }
}
void cpx_example_init(){
	cpx_ExampleHandle = osThreadNew(cpx_Example, NULL, &cpx_Example_attributes);
}


static const UserInit cpx_init = {
		.init = cpx_example_init,
};

//USER_INIT(cpx_init);
