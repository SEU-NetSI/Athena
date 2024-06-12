/*
 * tca6408a.h
 *
 *  Created on: Mar 25, 2024
 *      Author: Hanjie Yu
 */

#ifndef BSP_COMPONENTS_TCA6408A_INC_TCA6408A_H_
#define BSP_COMPONENTS_TCA6408A_INC_TCA6408A_H_

#include <stdbool.h>
// i2c ADDRESS
#define I2C_EXPANDER_DEFAULT_I2C_ADDRESS            ((uint8_t)0x20)
// Registers
#define OUTPUT_PORT_REG_ADDRESS                     ((uint8_t)0x01)
#define INPUT_PORT_REG_ADDRESS                      ((uint8_t)0x00)
#define CONFIGURATION_REG_ADDRESS                   ((uint8_t)0x03)
#define POLARITY_INVERSION_REG_ADDRESS              ((uint8_t)0x02)
// Pins
#define LPN_BACKWARD_PIN                            ((uint8_t)1<<0)
#define I2C_RST_BACKWARD_PIN                        ((uint8_t)1<<1)
#define LPN_FORWARD_PIN                             ((uint8_t)1<<2)
#define I2C_RST_FORWARD_PIN                         ((uint8_t)1<<3)
#define LED_FORWARD_PIN                             ((uint8_t)1<<4)
#define LED_BACKWARD_PIN                            ((uint8_t)1<<5)
#define INTERRUPT_SENSE_FORWARD_PIN                 ((uint8_t)1<<6)
#define INTERRUPT_SENSE_BACKWARD_PIN                ((uint8_t)1<<7)

#define LPN_BACKWARD_PIN_NUM                        ((uint8_t)0)
#define I2C_RST_BACKWARD_PIN_NUM                    ((uint8_t)1)
#define LPN_FORWARD_PIN_NUM                         ((uint8_t)2)
#define I2C_RST_FORWARD_PIN_NUM                     ((uint8_t)3)
#define LED_FORWARD_PIN_NUM                         ((uint8_t)4)
#define LED_BACKWARD_PIN_NUM                        ((uint8_t)5)
#define INTERRUPT_SENSE_FORWARD_PIN_NUM             ((uint8_t)6)
#define INTERRUPT_SENSE_BACKWARD_PIN_NUM            ((uint8_t)7)


bool I2C_expander_set_register(uint8_t reg_address,uint8_t reg_value);
bool I2C_expander_get_register(uint8_t reg_address,uint8_t* reg_value);
bool I2C_expander_set_output_pin(uint8_t pin_number,bool pin_state);
bool I2C_expander_get_input_pin(uint8_t pin_number,bool* pin_state);
bool I2C_expander_initialize();


#endif /* BSP_COMPONENTS_TCA6408A_INC_TCA6408A_H_ */
