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

void doubleToString(double num, char *str) {
    // 提取整数部分和小数部分
    long int integerPart = (long int)num;
    double decimalPart = num - integerPart;

    // 拼接整数部分
    int i = 0;
    if (integerPart == 0) {
        str[i++] = '0';  // 特殊处理整数部分为0的情况
    } else {
        int temp = integerPart;
        int digitCount = 0;
        while (temp > 0) {
            digitCount++;
            temp /= 10;
        }

        for (int j = digitCount - 1; j >= 0; j--) {
            str[i + j] = '0' + (integerPart % 10);
            integerPart /= 10;
        }
        i += digitCount;
    }

    str[i++] = '.';  // 添加小数点

    // 拼接小数部分，保留两位小数
    decimalPart *= 100;  // 获取小数点后两位
    int decimalInt = (int)decimalPart;

    str[i++] = '0' + (decimalInt / 10);  // 小数部分的十位
    str[i++] = '0' + (decimalInt % 10);  // 小数部分的个位

    str[i] = '\0';  // 字符串结束符
}


static void cpxPacketCallback(const CPXPacket_t *cpxRx)
{
	double pos[6];
	char result[50];
	memcpy((double *)pos, cpxRx->data, cpxRx->dataLength);
	DEBUG_PRINTF("Got Data from STM32: [");
	        for (size_t i = 0; i < 6; i++)
	        {
//	        	doubleToString(pos[i], result);
	            DEBUG_PRINTF("%f \n", pos[i]);
	            vTaskDelay(500);
	        }
	        DEBUG_PRINTF("]\n");
//	DEBUG_PRINTF("Got Data from STM32: [");
//	DEBUG_PRINTF("%d \n", cpxRx->dataLength);
//	        for (size_t i = 0; i < cpxRx->dataLength; i++)
//	        {
//	            DEBUG_PRINTF("%d,", cpxRx->data[i]);
//	        }
//	        DEBUG_PRINTF("]\n");
}

static void cpx_Example(void *argument)
{
	DEBUG_PRINTF("cpx task is up\n");
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
//	    DEBUG_PRINTF("send packet to cf(%u).\n",cpxPacket.data[0]);
  }
}
void cpx_example_init(){
	cpx_ExampleHandle = osThreadNew(cpx_Example, NULL, &cpx_Example_attributes);
}


static const UserInit cpx_init = {
		.init = cpx_example_init,
};

//USER_INIT(cpx_init);
