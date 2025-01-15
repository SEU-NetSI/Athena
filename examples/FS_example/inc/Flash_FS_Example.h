#include "FileSystem_Configuration.h"
#include "debug.h"
#include "tof_init.h"
#include "tof_get_data.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "cpx/cpx.h"
#include "cpx/uart_transport.h"
#include "cpx/router.h"
//#include "Flash_FS_Example.c"

static VL53L5CX_Configuration vl53l5dev_f;
static VL53L5CX_ResultsData vl53l5_res_f;
static QueueHandle_t cpxQueue;
static float Pos[6];

struct log_entry
{
	/* data */
	uint32_t timestamp;
//	int16_t distantce_mm[64];
//	uint8_t target_status[64];
	double pos[6];
};
struct log_entry entry;
struct ringfs fs;       //定义ringfs结构体变量fs，这就是一个文件系统的实例
static uint32_t id = 0;

static void cpxCallback(const CPXPacket_t *cpxRx);
void FS_Example(void *argument);
void Data_Init();
void Data_Gather(void *argument);
void Data_Read(void *argument);
