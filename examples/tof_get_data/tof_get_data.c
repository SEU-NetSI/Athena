#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "tof_init.h"
#include "tof_get_data.h"
#include "debug.h"

static VL53L5CX_Configuration vl53l5dev_f;
static VL53L5CX_ResultsData vl53l5_res_f;
static tof_packet pk;
static ToFdataCallback tofHandlerCallback = NULL;
void tof_get_data(void *argument);
void ToFRegisterMessageHandler(ToFdataCallback callback);

osThreadId_t TOF_exampleHandle;

const osThreadAttr_t tof_get_data_attributes = {
		.name = "tof_get_data",
		.stack_size = 500,
		.priority = (osPriority_t) osPriorityNormal,
};

void tof_init_fun(){
	TOF_exampleHandle = osThreadNew(tof_get_data, NULL, &tof_get_data_attributes);
}

static const UserInit tof_init = {
		.init = tof_init_fun,
};

//USER_INIT(tof_init);


void tof_get_data(void* argument){
	pk.id = 0;
#ifdef ZRANGER
	LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_3);
	LL_mDelay(100);
	initialize_sensors_I2C(&vl53l5dev_f,1);//初始化VL53L5CX传感器
	vl53l5cx_start_ranging(&vl53l5dev_f);//配置开始测距
#else
	I2C_expander_initialize();//初始化I2C多路复用器
	initialize_sensors_I2C(&vl53l5dev_f,1);//初始化VL53L5CX传感器
	vl53l5cx_start_ranging(&vl53l5dev_f);//配置开始测距
#endif
	while(1){
	  get_sensor_data(&vl53l5dev_f, &vl53l5_res_f);//获取传感器数据
	  memcpy(pk.distance, vl53l5_res_f.distance_mm, sizeof(pk.distance));
	  tofHandlerCallback(&pk);
      LL_GPIO_TogglePin(GPIOB, LL_GPIO_PIN_9);
	  LL_mDelay(500);
	}
}

void ToFRegisterMessageHandler(ToFdataCallback callback) {
    tofHandlerCallback = callback;
}
