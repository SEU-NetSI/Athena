#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "debug.h"
#include "cpx/cpx.h"
#include "cpx/uart_transport.h"
#include "cpx/router.h"
#include "Flash_FS_Example.h"
#include "tof_init.h"
#include "tof_get_data.h"

osThreadId_t cpx_ExampleHandle;
const osThreadAttr_t cpx_Example_attributes = {
		.name = "Data_Read",
		.stack_size = 2000,
		.priority = (osPriority_t) osPriorityNormal,
};

struct log_entry
{
	/* data */
	uint32_t timestamp;
	int16_t distantce_mm[64];
//	uint8_t target_status[64];
	double pos[6];
};
struct log_entry entry;
struct ringfs fs;       //定义ringfs结构体变量fs，这就是一个文件系统的实例
static uint32_t id = 0;
static tof_packet pk;
static bool flag = 0;
static CPXPacket_t cpxPacket;

void Data_Read(void *argument)
{
	ringfs_init(&fs, &flash, sizeof(struct log_entry));      //初始化文件系统

	if (ringfs_scan(&fs) != 0) {
		ringfs_format(&fs);     //格式化文件系统
	}

	DEBUG_PRINTF("begin to read data \n");
	struct log_entry entry1;
	for(int i = 0;i < 100;i++)
	{
		DEBUG_PRINTF("\n");
		assert(ringfs_fetch(&fs, &entry1) == 0);
		DEBUG_PRINTF("get data %d :[ \n", entry1.timestamp);
		for(int j = 0;j < 6;j++)
		{
			DEBUG_PRINTF("%f\n", entry1.pos[j]);
		}
		DEBUG_PRINTF("]\n");
		DEBUG_PRINTF("get data from tof \n");
		for(int j=0;j < 64;j++)
		{
			DEBUG_PRINTF("%d, ", entry1.distantce_mm[j]);
			if(j % 8 == 0)DEBUG_PRINTF("\n");
		}
		vTaskDelay(1000);
	}

	ringfs_discard(&fs);

	while(true)
	{
		vTaskDelay(100);
	}
}

static void cpxCallback(const CPXPacket_t *cpxRx)
{
	if(flag)return;
	if(cpxRx->dataLength == 1 && cpxRx->data[0] == 0xFE)
	{
		flag = 1;
	}
	memcpy((double *)entry.pos, cpxRx->data, cpxRx->dataLength);
	entry.timestamp = id++;
	DEBUG_PRINTF("Got data %d from st32 \n", id);
	for (size_t i = 0; i < 6; i++)
	{
		DEBUG_PRINTF("%f \n", entry.pos[i]);
		vTaskDelay(10);
	}
	for(int i=0;i<64;i++)
	{
		DEBUG_PRINTF("%d, ", entry.distantce_mm[i]);
	    if(i % 8 == 0)DEBUG_PRINTF("\n");
	}
	DEBUG_PRINTF("]\n");
	ringfs_append(&fs, &entry);
	vTaskDelay(5);
}

static void tofCallback()
{
	if(flag)
	{
		DEBUG_PRINTF("over! \n");
		return;
	}
	pk = get_pk();
	memcpy(entry.distantce_mm, pk.distance, sizeof(entry.distantce_mm));
	vTaskDelay(5);
}

void Data_Write()
{
	ringfs_init(&fs, &flash, sizeof(struct log_entry));      //初始化文件系统

	if (ringfs_scan(&fs) != 0) {
		ringfs_format(&fs);     //格式化文件系统
	}
	vTaskDelay(500);
	cpxRegisterAppMessageHandler(cpxCallback);
	ToFRegisterMessageHandler(tofCallback);
	DEBUG_PRINTF("to call uart_transport_init\n");
	uart_transport_init();
	routerInit();
	DEBUG_PRINTF("router initialized\n");
	cpxInit();
	for(int i=0;i<5;i++)
	{
		osDelay(50);
		cpxInitRoute(CPX_T_GAP8, CPX_T_STM32, CPX_F_APP, &cpxPacket.route);
		cpxPacket.dataLength = 1;
		cpxPacket.data[0]= 0xEF;
		cpxSendPacketBlocking(&cpxPacket);
		//DEBUG_PRINTF("send packet to cf(%u).\n",cpxPacket.data[0]);
	}
	while(1)
	{
		osDelay(50);

	}
}

//static void cpx_Example(void *argument)
//{
//	DEBUG_PRINTF("cpx example task is up\n");
//	cpxRegisterAppMessageHandler(cpxPacketCallback);
//	uart_transport_init();
//	routerInit();
//	DEBUG_PRINTF("router initialized\n");
//	cpxInit();
//	uint8_t count=0;
//  while(1)
//  {
//	    vTaskDelay(2000);
////	    int mark = uxTaskGetStackHighWaterMark(cpx_ExampleHandle);
//	    cpxInitRoute(CPX_T_GAP8, CPX_T_STM32, CPX_F_APP, &cpxPacket.route);
//	    cpxPacket.dataLength = 1;
//	    cpxPacket.data[0]=count;
//	    count++;
//	    cpxSendPacketBlocking(&cpxPacket);
//	    //DEBUG_PRINTF("send packet to cf(%u).\n",cpxPacket.data[0]);
//  }
//}
void cpx_example_init(){
//	cpx_ExampleHandle = osThreadNew(Data_Write, NULL, &cpx_Example_attributes);
	cpx_ExampleHandle = osThreadNew(Data_Read, NULL, &cpx_Example_attributes);
}


static const UserInit cpx_init = {
		.init = cpx_example_init,
};


//USER_INIT(cpx_init);
