/*
 * multi_tof.c
 *
 *  Created on: May 28, 2024
 *      Author: Hanjie Yu
 */

#include "test_tof.h"



#define VL53L5CX_FORWARD_I2C_ADDRESS            ((uint16_t)(VL53L5CX_DEFAULT_I2C_ADDRESS*4))
#define VL53L5CX_BACKWARD_I2C_ADDRESS            ((uint16_t)(VL53L5CX_FORWARD_I2C_ADDRESS+2))


bool config_sensors(VL53L5CX_Configuration *p_dev, uint16_t new_i2c_address)
{
  p_dev->platform.address = VL53L5CX_DEFAULT_I2C_ADDRESS; // use default adress for first use

  // initialize the sensor
  uint8_t tof_res = vl53l5cx_init(p_dev);   if (tof_res != VL53L5CX_STATUS_OK) return false ;
  //DEBUG_PRINT("ToF Config Result: %d \n", tof_init_res);

  // Configurations
  //change i2c address
  tof_res = vl53l5cx_set_i2c_address(p_dev, new_i2c_address);if (tof_res != VL53L5CX_STATUS_OK) return false ;
  tof_res = vl53l5cx_set_resolution(p_dev, VL53L5CX_RESOLUTION_8X8);if (tof_res != VL53L5CX_STATUS_OK) return false ;
  // 1Hz
  tof_res = vl53l5cx_set_ranging_frequency_hz(p_dev, 10);if (tof_res != VL53L5CX_STATUS_OK) return false ;
  tof_res = vl53l5cx_set_target_order(p_dev, VL53L5CX_TARGET_ORDER_CLOSEST);if (tof_res != VL53L5CX_STATUS_OK) return false ;
  tof_res = vl53l5cx_set_ranging_mode(p_dev, VL53L5CX_RANGING_MODE_CONTINUOUS);if (tof_res != VL53L5CX_STATUS_OK) return false ;
  //tof_res = vl53l5cx_set_ranging_mode(p_dev, VL53L5CX_RANGING_MODE_AUTONOMOUS);if (tof_res != VL53L5CX_STATUS_OK) return false ;// TODO test it

  //Check for sensor to be alive
  uint8_t isAlive;
  tof_res =vl53l5cx_is_alive(p_dev,&isAlive);if (tof_res != VL53L5CX_STATUS_OK) return false;
  if (isAlive != 1) return false;

  // All Good!
  return true;
}


bool initialize_sensors_I2C(VL53L5CX_Configuration *p_dev, uint8_t mode)
{
  bool status = false;

  //reset I2C  //configure pins out/in for forward only

  //status = I2C_expander_set_register(OUTPUT_PORT_REG_ADDRESS,I2C_RST_BACKWARD_PIN,I2C_RST_FORWARD_PIN);if (status == false)return status;

  if (mode == 1 && p_dev != NULL){
    //enable forward only and config
    status = I2C_expander_set_register(OUTPUT_PORT_REG_ADDRESS,LPN_FORWARD_PIN | LED_FORWARD_PIN );if (status == false)return status;
    status = config_sensors(p_dev,VL53L5CX_FORWARD_I2C_ADDRESS);if (status == false)return status;
  }
  if (mode == 2 && p_dev != NULL){
    //enable backward only and config
    status = I2C_expander_set_register(OUTPUT_PORT_REG_ADDRESS,LPN_BACKWARD_PIN | LED_BACKWARD_PIN); if (status == false)return status;
    status = config_sensors(p_dev,VL53L5CX_BACKWARD_I2C_ADDRESS);if (status == false)return status;
  }
  //status = I2C_expander_set_register(OUTPUT_PORT_REG_ADDRESS,0x00); //all off
  if (mode == 3){
    //enable both forward & backward
    status = I2C_expander_set_register(OUTPUT_PORT_REG_ADDRESS,LPN_BACKWARD_PIN | LED_BACKWARD_PIN|LPN_FORWARD_PIN | LED_FORWARD_PIN); if (status == false)return status;
  }
  return status;
}

bool get_sensor_data(VL53L5CX_Configuration *p_dev,VL53L5CX_ResultsData *p_results){

  // Check  for data ready I2c
  uint8_t ranging_ready = 2;
  //ranging_ready --> 0 if data is not ready, or 1 if a new data is ready.
  uint8_t status = vl53l5cx_check_data_ready(p_dev, &ranging_ready);if (status != VL53L5CX_STATUS_OK) return false;

  // 1 Get data in case it is ready
  if (ranging_ready == 1){
    status = vl53l5cx_get_ranging_data(p_dev, p_results);if (status != VL53L5CX_STATUS_OK) return false;
  }else {
    //0  data in not ready yet
    return false;
  }

  // All good then
  //return false;// TODO deleet
  return true;
}

