/*******************************************************************************
* Copyright (c) 2020, STMicroelectronics - All Rights Reserved
*
* This file is part of the VL53L5CX Ultra Lite Driver and is dual licensed,
* either 'STMicroelectronics Proprietary license'
* or 'BSD 3-clause "New" or "Revised" License' , at your option.
*
********************************************************************************
*
* 'STMicroelectronics Proprietary license'
*
********************************************************************************
*
* License terms: STMicroelectronics Proprietary in accordance with licensing
* terms at www.st.com/sla0081
*
* STMicroelectronics confidential
* Reproduction and Communication of this document is strictly prohibited unless
* specifically authorized in writing by STMicroelectronics.
*
*
********************************************************************************
*
* Alternatively, the VL53L5CX Ultra Lite Driver may be distributed under the
* terms of 'BSD 3-clause "New" or "Revised" License', in which case the
* following provisions apply instead of the ones mentioned above :
*
********************************************************************************
*
* License terms: BSD 3-clause "New" or "Revised" License.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice, this
* list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright notice,
* this list of conditions and the following disclaimer in the documentation
* and/or other materials provided with the distribution.
*
* 3. Neither the name of the copyright holder nor the names of its contributors
* may be used to endorse or promote products derived from this software
* without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*
*******************************************************************************/

#include "platform.h"
#include "i2c_drv.h"
#include "cmsis_os.h"

#define chunk_size 252


uint8_t RdByte(
		VL53L5CX_Platform *p_platform,
		uint16_t RegisterAdress,
		uint8_t *p_value)
{
	uint16_t Address= (p_platform->address)>>1;
	i2cdevReadReg16(I2C3, Address, RegisterAdress, p_value, 1);
	return 0;
}

uint8_t WrByte(
		VL53L5CX_Platform *p_platform,
		uint16_t RegisterAdress,
		uint8_t value)
{
	uint8_t rx_buffer[1];
	rx_buffer[0]=value;
	uint16_t Address= (p_platform->address)>>1;
	i2cdevWriteReg16(I2C3, Address, RegisterAdress, rx_buffer, 1);
	return 0;
}

uint8_t WrMulti(
		VL53L5CX_Platform *p_platform,
		uint16_t RegisterAdress,
		uint8_t *p_values,
		uint32_t size)
{
	uint16_t Address= (p_platform->address)>>1;
	uint32_t remaining_size = size;  // Calculate remaining size to write
    uint16_t current_address =RegisterAdress;  // Initialize current address
	while(remaining_size> 0){
		uint32_t current_chunk_size = (remaining_size > chunk_size) ? chunk_size : remaining_size;
		i2cdevWriteReg16(I2C3, Address, current_address, p_values, current_chunk_size);
		remaining_size-= current_chunk_size;
		current_address+=current_chunk_size;
		p_values+= current_chunk_size;
		osDelay(2);
	}

	return 0;
}

//uint8_t WrMulti(
//		VL53L5CX_Platform *p_platform,
//		uint16_t RegisterAdress,
//		uint8_t *p_values,
//		uint32_t size)
//{
//	uint16_t Address= (p_platform->address)>>1;
//	I2C_Transmit(I2C3, Address, RegisterAdress, p_values, size);
//	return 0;
//}

uint8_t RdMulti(
		VL53L5CX_Platform *p_platform,
		uint16_t RegisterAdress,
		uint8_t *p_values,
		uint32_t size)
{
	uint16_t Address= (p_platform->address)>>1;
	uint32_t remaining_size = size;  // Calculate remaining size to write
	uint16_t current_address =RegisterAdress;  // Initialize current address
	while(remaining_size> 0){
		uint32_t current_chunk_size = (remaining_size > chunk_size) ? chunk_size : remaining_size;
		i2cdevReadReg16(I2C3, Address, current_address, p_values, current_chunk_size);
		remaining_size-= current_chunk_size;
		current_address+=current_chunk_size;
		p_values+= current_chunk_size;
		LL_mDelay(2);
	}
	return 0;
}

uint8_t Reset_Sensor(VL53L5CX_Platform *p_platform)
{
	/* (Optional) Need to be implemented by customer. This function returns 0 if OK */

	/* Set pin LPN to LOW */
	/* Set pin AVDD to LOW */
	/* Set pin VDDIO  to LOW */
	WaitMs(p_platform, 100);

	/* Set pin LPN of to HIGH */
	/* Set pin AVDD of to HIGH */
	/* Set pin VDDIO of  to HIGH */
	WaitMs(p_platform, 100);
  
	return 0;
}

void SwapBuffer(
		uint8_t 		*buffer,
		uint16_t 	 	 size)
{
	uint32_t i, tmp;

	/* Example of possible implementation using <string.h> */
	for(i = 0; i < size; i = i + 4)
	{
		tmp = (
		  buffer[i]<<24)
		|(buffer[i+1]<<16)
		|(buffer[i+2]<<8)
		|(buffer[i+3]);

		memcpy(&(buffer[i]), &tmp, 4);
	}
}

uint8_t WaitMs(
		VL53L5CX_Platform *p_platform,
               uint32_t TimeMs)
{
	osDelay(TimeMs);
	return 0;
}
