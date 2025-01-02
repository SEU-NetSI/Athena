#include "tof_init.h"
#include "tof_get_data.h"

static VL53L5CX_Configuration vl53l5dev_f;
static VL53L5CX_ResultsData vl53l5_res_f;

void tof_get_data(void){
	I2C_expander_initialize();
	initialize_sensors_I2C(&vl53l5dev_f,1);
	vl53l5cx_start_ranging(&vl53l5dev_f);
	while(1){
	  	get_sensor_data(&vl53l5dev_f, &vl53l5_res_f);
        LL_mDelay(100);
	}
}