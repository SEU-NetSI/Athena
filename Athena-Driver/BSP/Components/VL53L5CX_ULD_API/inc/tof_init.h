/*
 * tof_init.h
 *
 *  Created on: Apr 25, 2024
 *      Author: mfxjx
 */

#include "i2c_drv.h"
#include "tca6408a.h"
#include "vl53l5cx_api.h"
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

bool config_sensors(VL53L5CX_Configuration *p_dev, uint16_t new_i2c_address);
bool initialize_sensors_I2C(VL53L5CX_Configuration *p_dev, uint8_t mode);
bool get_sensor_data(VL53L5CX_Configuration *p_dev,VL53L5CX_ResultsData *p_results);
