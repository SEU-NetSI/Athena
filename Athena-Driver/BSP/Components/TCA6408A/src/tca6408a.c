/*
 * tca6408a.c
 *
 *  Created on: Mar 25, 2024
 *      Author: Hanjie Yu
 */

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "i2c_drv.h"

#include "tca6408a.h"

bool I2C_expander_set_register(uint8_t reg_address,uint8_t reg_value)
{
	i2cdevWriteReg8(I2C3, I2C_EXPANDER_DEFAULT_I2C_ADDRESS, reg_address, reg_value);

	return true;
}

bool I2C_expander_get_register(uint8_t reg_address,uint8_t* reg_value)
{
	*reg_value=i2cdevReadReg8(I2C3, I2C_EXPANDER_DEFAULT_I2C_ADDRESS, reg_address);

	return true;
}

bool I2C_expander_set_output_pin(uint8_t pin_number,bool pin_state)
{
    uint8_t output_reg_value;

    //read output current value
    output_reg_value= i2cdevReadReg8(I2C3, I2C_EXPANDER_DEFAULT_I2C_ADDRESS, OUTPUT_PORT_REG_ADDRESS);

    //check if the pin is in the same status // no need to rewrite it
    if ((output_reg_value &  1<<pin_number )== pin_state)
        return true;

    //mask selected pin
    if (pin_state == true)
        output_reg_value |= (1<<pin_number);
    else
        output_reg_value &= ~(1<<pin_number);

    //update reg value
    i2cdevWriteReg8(I2C3, I2C_EXPANDER_DEFAULT_I2C_ADDRESS, OUTPUT_PORT_REG_ADDRESS, output_reg_value);

    return true;
}

bool I2C_expander_toggle_output_pin(uint8_t pin_number)
{
    uint8_t output_reg_value;

    //read output current value
    output_reg_value= i2cdevReadReg8(I2C3, I2C_EXPANDER_DEFAULT_I2C_ADDRESS, OUTPUT_PORT_REG_ADDRESS);

    //mask selected pin
    if ((output_reg_value &  1<<pin_number )== false)
        output_reg_value |= (1<<pin_number);
    else
        output_reg_value &= ~(1<<pin_number);

    //update reg value
    i2cdevWriteReg8(I2C3, I2C_EXPANDER_DEFAULT_I2C_ADDRESS, OUTPUT_PORT_REG_ADDRESS, output_reg_value);

    return true;
}

bool I2C_expander_get_input_pin(uint8_t pin_number,bool* pin_state)
{
    uint8_t input_reg_value;

    //read input current value
    input_reg_value= i2cdevReadReg8(I2C3, I2C_EXPANDER_DEFAULT_I2C_ADDRESS,INPUT_PORT_REG_ADDRESS);
    //mask selected pin
    *pin_state = ((input_reg_value&(1<<pin_number))) ? true : false;

    return true;
}

bool I2C_expander_initialize()
{
    uint8_t reg_value;

    //set all outputs zero
    reg_value = 0x30; // 1-->on, 0-->off
    I2C_expander_set_register(OUTPUT_PORT_REG_ADDRESS,0x00);

    //configure inversion all 0
    reg_value = 0x00; // 1-->inveritng, 0-->non-inveritng
    i2cdevWriteReg8(I2C3, I2C_EXPANDER_DEFAULT_I2C_ADDRESS, POLARITY_INVERSION_REG_ADDRESS, reg_value);

    //configure pins out/in
    reg_value = INTERRUPT_SENSE_BACKWARD_PIN|INTERRUPT_SENSE_FORWARD_PIN; // 1-->input, 0-->output
    i2cdevWriteReg8(I2C3, I2C_EXPANDER_DEFAULT_I2C_ADDRESS, CONFIGURATION_REG_ADDRESS, reg_value);

    reg_value=LPN_FORWARD_PIN | LED_FORWARD_PIN;
    I2C_expander_set_register(OUTPUT_PORT_REG_ADDRESS,LPN_FORWARD_PIN | LED_FORWARD_PIN );

    return true;
}

