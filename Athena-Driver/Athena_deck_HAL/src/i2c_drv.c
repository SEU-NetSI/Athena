/*
 * i2c_drv.c
 * Created on: Mar 16, 2024
 * Author: Hanjie Yu
 * Project: Athena-deck I2C general driver
 * TBD:
 * 1. MTU: split data
 * 2. DMA: IT_handler
 * 3. I2C: Error_Handler
 * 4. Code: Merge the code xxReg8 & xxReg16
 * 5. Debug: osDelay & Print
 * ****************************************
 * Version Control:
 * v2.0@2024.04.27
 * 1. i2c Read and Write with DMA: Test OK
 * 2. Register address 8bit and 16 bit support
 * */
#include <stdio.h>

#include "stm32l4xx.h"
#include "stm32l4xx_ll_i2c.h"
#include "i2c_drv.h"
#include "stm32l4xx_ll_dma.h"
#include "retarget.h"

#define  DUAL_BUFFER

#define BUFFER_SIZE 255

uint8_t buffer1[BUFFER_SIZE];
uint8_t buffer2[BUFFER_SIZE];
extern volatile uint8_t dma_transfer_complete;
extern volatile uint8_t current_buffer;

uint8_t i2cdevReadReg8(I2C_TypeDef *I2Cx , uint8_t SlaveAddr_IC , uint8_t target_reg);
ErrorStatus i2cdevWriteReg8(I2C_TypeDef *I2Cx , uint8_t SlaveAddr_IC , uint8_t target_reg , uint8_t value);
void i2cdevReadReg_Mul(I2C_TypeDef *I2Cx , uint8_t SlaveAddr_IC , uint8_t target_reg, uint8_t size, uint8_t *rx_buffer);
ErrorStatus i2cdevWriteReg_Mul(I2C_TypeDef *I2Cx, uint8_t SlaveAddr_IC, uint8_t target_reg, uint8_t size, uint8_t *tx_buffer);
uint8_t *i2cdevReadRegSeq_DMA(I2C_TypeDef *I2Cx, uint8_t SlaveAddr_IC, uint8_t target_reg, uint8_t *rx_buffer, uint16_t size, DMA_Callback callback);
ErrorStatus i2cdevWriteRegSeq_DMA();
void i2cdevReadReg16(I2C_TypeDef *I2Cx ,
						uint8_t SlaveAddr_IC ,
						uint16_t target_reg,
						uint8_t *p_values,
						uint32_t size);
void i2cdevWriteReg16(I2C_TypeDef *I2Cx,
								uint8_t SlaveAddr_IC,
								uint16_t target_reg,
								uint8_t *p_value,
								uint32_t size);
uint8_t *i2cdevReadRegSeq_DMA16(I2C_TypeDef *I2Cx, uint8_t SlaveAddr_IC, uint16_t target_reg, uint8_t *rx_buffer, uint16_t size, DMA_Callback callback);
void I2C_Transmit(	I2C_TypeDef *I2Cx,
					uint8_t SlaveAddr_IC,
					uint16_t target_reg,
					uint8_t *p_values,
					uint32_t Length);

//Function Pointer
typedef void (*DMA_Callback)(void);

//Callback Definition
DMA_Callback I2C_DMA_RX_callback = NULL;

/*
 * @brief  Read 1 byte from I2C slave's Register
 * @param  I2C_TypeDef *I2Cx  -- Number port I2C
  		   SlaveAddr_IC - Address Slave IC on bus I2C
  		   target_reg -target register need to read from
 * @retval return value read from REG
 * */

uint8_t i2cdevReadReg8(I2C_TypeDef *I2Cx , uint8_t SlaveAddr_IC , uint8_t target_reg)
{
	uint8_t reg_value;
	//Address process
	SlaveAddr_IC = SlaveAddr_IC<<1;

	//I2C wait for : BUS BUSY
	uint32_t counter = 0;
	while(LL_I2C_IsActiveFlag_BUSY(I2Cx) == SET){
		counter++;
		if( counter == 25000 ) { //150ms
			Error_Handler();
			return 0xFF;
		}
	}

	//Write the TARGET REGISTER to the I2C slave
	LL_I2C_HandleTransfer(I2Cx , SlaveAddr_IC , LL_I2C_ADDRSLAVE_7BIT , 1 , LL_I2C_MODE_SOFTEND , LL_I2C_GENERATE_START_WRITE); //LL_I2C_GENERATE_START_READ

	//I2C wait for : TX REG BUSY
	while(LL_I2C_IsActiveFlag_TXE(I2Cx)==RESET);

	LL_I2C_TransmitData8(I2Cx, target_reg);

	//I2C wait for : TX REG BUSY AND CLEAR
	counter = 0;
	while(LL_I2C_IsActiveFlag_TXE(I2Cx)==RESET){
		counter++;
		if( counter == 25000){ //150ms
			LL_I2C_ClearFlag_TXE(I2Cx);
			Error_Handler();
			return 0xFF;
		}
	}

	//I2C wait for : TX COMPLETE
	while(LL_I2C_IsActiveFlag_TC(I2Cx)==RESET);

	LL_I2C_HandleTransfer(I2Cx, SlaveAddr_IC, LL_I2C_ADDRSLAVE_7BIT, 1 ,LL_I2C_MODE_AUTOEND ,LL_I2C_GENERATE_START_READ);

	//I2C wait for : RX REG BUSY
	while(LL_I2C_IsActiveFlag_RXNE(I2Cx)==RESET);

	reg_value = LL_I2C_ReceiveData8(I2Cx);

	//I2C wait for : STOP DETECT
	while(LL_I2C_IsActiveFlag_STOP(I2Cx)==RESET);

	LL_I2C_ClearFlag_STOP(I2Cx);

	return reg_value;
}

/*
 * @brief  Write 1 byte to I2C slave's Register
 * @param  I2C_TypeDef *I2Cx  -- Number port I2C
  		   SlaveAddr_IC - Address Slave IC on bus I2C
  		   target_reg -target register need to write to
  		   value - value need to write to REG
 * @return return An ErrorStatus enumeration
 *         - SUCCESS:
 *         - ERROR:   Not applicable
 * */

ErrorStatus i2cdevWriteReg8(I2C_TypeDef *I2Cx , uint8_t SlaveAddr_IC , uint8_t target_reg , uint8_t value)
{
	//Address process
	SlaveAddr_IC = SlaveAddr_IC<<1;

	//I2C wait for : BUS BUSY
	uint32_t counter = 0;
	while(LL_I2C_IsActiveFlag_BUSY(I2Cx) == SET){
		counter++;
		if( counter == 25000 ) {
			Error_Handler();
			return 0xFF;
		}
	}

	LL_I2C_HandleTransfer(I2Cx, SlaveAddr_IC, LL_I2C_ADDRSLAVE_7BIT, 2, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_WRITE);

	//I2C wait for : TX REG BUSY AND CLEAR
	while(LL_I2C_IsActiveFlag_TXE(I2Cx)==RESET);

    LL_I2C_TransmitData8(I2Cx, target_reg);

	//I2C wait for : TX REG BUSY AND CLEAR
    counter=0;
    while(LL_I2C_IsActiveFlag_TXE(I2Cx)==RESET){
        counter++;
        if( counter == 25000 ){
            LL_I2C_ClearFlag_TXE(I2Cx);
            Error_Handler();
            return ERROR;
        }
    }

    LL_I2C_TransmitData8(I2Cx, value);

	//I2C wait for : TX REG BUSY AND CLEAR
    while(LL_I2C_IsActiveFlag_TXE(I2Cx)==RESET);

    //I2C wait for : STOP CLEAR
    while(LL_I2C_IsActiveFlag_STOP(I2Cx)==RESET);

    LL_I2C_ClearFlag_STOP(I2Cx);

    return SUCCESS;
}

/*
 * I2C Multi Byte Read Test Code
 * TBD: DELETED
 * */
void i2cdevReadReg_Mul(I2C_TypeDef *I2Cx , uint8_t SlaveAddr_IC , uint8_t target_reg, uint8_t size, uint8_t *rx_buffer)
{
	//Address process
	SlaveAddr_IC = SlaveAddr_IC<<1;

	//I2C wait for : BUS BUSY
	uint32_t counter = 0;
	while(LL_I2C_IsActiveFlag_BUSY(I2Cx) == SET){
		counter++;
		if( counter == 25000 ) { //150ms
			Error_Handler();
		}
	}

	//Write the TARGET REGISTER to the I2C slave
	LL_I2C_HandleTransfer(I2Cx , SlaveAddr_IC , LL_I2C_ADDRSLAVE_7BIT , 1 , LL_I2C_MODE_SOFTEND , LL_I2C_GENERATE_START_WRITE); //LL_I2C_GENERATE_START_READ

	//I2C wait for : TX REG BUSY
	while(LL_I2C_IsActiveFlag_TXE(I2Cx)==RESET);

	LL_I2C_TransmitData8(I2Cx, target_reg);

	LL_I2C_ClearFlag_STOP(I2Cx);

	//I2C wait for : TX REG BUSY AND CLEAR
	counter = 0;
	while(LL_I2C_IsActiveFlag_TXE(I2Cx)==RESET){
		counter++;
		if( counter == 25000){ //150ms
			LL_I2C_ClearFlag_TXE(I2Cx);
			Error_Handler();
		}
	}

	//I2C wait for : TX COMPLETE
	while(LL_I2C_IsActiveFlag_TC(I2Cx)==RESET);

//	//I2C wait for : RX REG BUSY
//	while(LL_I2C_IsActiveFlag_RXNE(I2Cx)==RESET);



	// Receive multiple bytes of data
	for (uint8_t i = 0; i < size; ++i) {
		LL_I2C_HandleTransfer(I2Cx, SlaveAddr_IC, LL_I2C_ADDRSLAVE_7BIT, 1 ,LL_I2C_MODE_SOFTEND ,LL_I2C_GENERATE_START_READ);


		rx_buffer[i] = LL_I2C_ReceiveData8(I2Cx);

		//I2C wait for : STOP DETECT

		LL_I2C_ClearFlag_STOP(I2Cx);

	}

}

/*
 * I2C Multi Byte Write Code
 * TBD: MERGE
 * */
ErrorStatus i2cdevWriteReg_Mul(I2C_TypeDef *I2Cx, uint8_t SlaveAddr_IC, uint8_t target_reg, uint8_t size, uint8_t *tx_buffer)
{
	//Address process
	SlaveAddr_IC = SlaveAddr_IC<<1;

	//I2C wait for : BUS BUSY
	uint32_t counter = 0;
	while(LL_I2C_IsActiveFlag_BUSY(I2Cx) == SET){
		counter++;
		if( counter == 25000 ) {
			Error_Handler();
			return 0xFF;
		}
	}

	LL_I2C_HandleTransfer(I2Cx, SlaveAddr_IC, LL_I2C_ADDRSLAVE_7BIT, 1+size, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_WRITE);

	//I2C wait for : TX REG BUSY AND CLEAR
	while(LL_I2C_IsActiveFlag_TXE(I2Cx)==RESET);

    LL_I2C_TransmitData8(I2Cx, target_reg);

	//I2C wait for : TX REG BUSY AND CLEAR
    counter=0;
    while(LL_I2C_IsActiveFlag_TXE(I2Cx)==RESET){
        counter++;
        if( counter == 25000 ){
            LL_I2C_ClearFlag_TXE(I2Cx);
            Error_Handler();
            return ERROR;
        }
    }

    for(int i=0;i<size;i++){

    	LL_I2C_TransmitData8(I2C1, tx_buffer[i]);
    }

	//I2C wait for : TX REG BUSY AND CLEAR
    while(LL_I2C_IsActiveFlag_TXE(I2Cx)==RESET);

    //I2C wait for : STOP CLEAR
    while(LL_I2C_IsActiveFlag_STOP(I2Cx)==RESET);

    LL_I2C_ClearFlag_STOP(I2Cx);

    return SUCCESS;
}

/*
 * @brief  Read MULTI byte from I2C slave's Register using DMA
 * @param  I2C_TypeDef *I2Cx  -- Number port I2C
  		   SlaveAddr_IC - Address Slave IC on bus I2C
  		   target_reg -target register need to read from
  		   rx_buffer - place the data read
  		   size - read data size
  		   callback - DMA callback function user defined
 * @retval return value read from REG
 * TBD: MERGE
 * */
uint8_t *i2cdevReadRegSeq_DMA(I2C_TypeDef *I2Cx, uint8_t SlaveAddr_IC, uint8_t target_reg, uint8_t *rx_buffer, uint16_t size, DMA_Callback callback)
{
	I2C_DMA_RX_callback = callback;
	//Address process
	SlaveAddr_IC = SlaveAddr_IC<<1;

	//I2C wait for : BUS BUSY
	uint32_t counter = 0;
	while(LL_I2C_IsActiveFlag_BUSY(I2Cx) == SET){
		counter++;
		if( counter == 25000 ) { //150ms
			Error_Handler();
			return NULL;
		}
	}

	//Write the TARGET REGISTER to the I2C slave
	LL_I2C_HandleTransfer(I2Cx , SlaveAddr_IC , LL_I2C_ADDRSLAVE_7BIT , 1 , LL_I2C_MODE_SOFTEND , LL_I2C_GENERATE_START_WRITE); //LL_I2C_GENERATE_START_READ

	//I2C wait for : TX REG BUSY
	while(LL_I2C_IsActiveFlag_TXE(I2Cx)==RESET);

	LL_I2C_TransmitData8(I2Cx, target_reg);

	//I2C wait for : TX REG BUSY AND CLEAR
	counter = 0;
	while(LL_I2C_IsActiveFlag_TXE(I2Cx)==RESET){
		counter++;
		if( counter == 25000){ //150ms
			LL_I2C_ClearFlag_TXE(I2Cx);
			Error_Handler();
			return NULL;
		}
	}

	//I2C wait for : TX COMPLETE
	while(LL_I2C_IsActiveFlag_TC(I2Cx)==RESET);

	LL_I2C_ClearFlag_STOP(I2Cx);

	LL_I2C_HandleTransfer(I2Cx, SlaveAddr_IC, LL_I2C_ADDRSLAVE_7BIT, size ,LL_I2C_MODE_AUTOEND ,LL_I2C_GENERATE_START_READ);

	//I2C DMA Transfer
	LL_DMA_ConfigAddresses(DMA1,
						   LL_DMA_CHANNEL_7,
						   LL_I2C_DMA_GetRegAddr(I2C1,LL_I2C_DMA_REG_DATA_RECEIVE),
						   (uint32_t)rx_buffer,
						   LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_7, size);
    LL_I2C_EnableDMAReq_RX(I2C1);
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_7);
    //Debug problem :	TBD
    osDelay(10);
    //TBD: double check if need Stop clear here.
    LL_I2C_ClearFlag_STOP(I2Cx);
    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_7);

    return rx_buffer;
}

/*
 * Test Code
 * */
void i2cdevReadReg16(I2C_TypeDef *I2Cx ,
						uint8_t SlaveAddr_IC ,
						uint16_t target_reg,
						uint8_t *p_values,
						uint32_t size){

	//Address process
	SlaveAddr_IC = SlaveAddr_IC<<1;

	LL_I2C_IsActiveFlag_BUSY(I2Cx);
	LL_I2C_HandleTransfer(I2Cx , SlaveAddr_IC , LL_I2C_ADDRSLAVE_7BIT , 2 , LL_I2C_MODE_AUTOEND , LL_I2C_GENERATE_START_WRITE);
	//Send the Address
	LL_I2C_TransmitData8(I2Cx, (uint8_t)(target_reg >> 8));

	while (!LL_I2C_IsActiveFlag_TXE(I2Cx));

	LL_I2C_TransmitData8(I2Cx, (uint8_t)(target_reg & 0xFF));

	while (!LL_I2C_IsActiveFlag_TXE(I2Cx));

	//Send the Data
	LL_I2C_HandleTransfer(I2Cx, SlaveAddr_IC, LL_I2C_ADDRSLAVE_7BIT, size ,LL_I2C_MODE_AUTOEND ,LL_I2C_GENERATE_START_READ);

	//I2C DMA Transfer
	LL_DMA_ConfigAddresses(DMA1,
						   LL_DMA_CHANNEL_3,
						   LL_I2C_DMA_GetRegAddr(I2Cx,LL_I2C_DMA_REG_DATA_RECEIVE),
						   (uint32_t)p_values,
						   LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_3, size);
    LL_I2C_EnableDMAReq_RX(I2Cx);
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_3);

    //Debug problem :TBD
    LL_mDelay(6);
    while(!LL_DMA_IsActiveFlag_TC3(DMA1));
    //TBD: double check if need Stop clear here.
    LL_I2C_ClearFlag_STOP(I2Cx);
    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_3);

}

void i2cdevWriteReg16(I2C_TypeDef *I2Cx,
                      uint8_t SlaveAddr_IC,
                      uint16_t target_reg,
                      uint8_t *p_values,
                      uint32_t size) {
    // Shift the Slave Address left by 1
    SlaveAddr_IC = SlaveAddr_IC << 1;

    // Ensure the I2C bus is not busy before starting a new transfer
    while (LL_I2C_IsActiveFlag_BUSY(I2Cx));

    // Begin the I2C write transfer to send the target register address
    LL_I2C_HandleTransfer(I2Cx, SlaveAddr_IC, LL_I2C_ADDRSLAVE_7BIT, 2+size, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_WRITE);

    // Send the high byte of the register address
    LL_I2C_TransmitData8(I2Cx, (uint8_t)(target_reg >> 8));

    // Wait until the data transmission is complete
    while (!LL_I2C_IsActiveFlag_TXE(I2Cx));

    // Send the low byte of the register address
    LL_I2C_TransmitData8(I2Cx, (uint8_t)(target_reg & 0xFF));

    // Wait until the data transmission is complete
    while (!LL_I2C_IsActiveFlag_TXE(I2Cx));
//    LL_I2C_TransmitData8(I2Cx, *p_values);
//
//    //I2C wait for : TX REG BUSY AND CLEAR
//    while(LL_I2C_IsActiveFlag_TXE(I2Cx)==RESET);
//    //I2C wait for : STOP CLEAR
//    while(LL_I2C_IsActiveFlag_STOP(I2Cx)==RESET);


    // Initiate the I2C transfer to send data with DMA
    LL_DMA_ConfigAddresses(DMA1,
                           LL_DMA_CHANNEL_2,
                           (uint32_t)p_values,
                           LL_I2C_DMA_GetRegAddr(I2Cx, LL_I2C_DMA_REG_DATA_TRANSMIT),
                           LL_DMA_DIRECTION_MEMORY_TO_PERIPH);

    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_2, size);
    LL_I2C_EnableDMAReq_TX(I2Cx);
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_2);

    osDelay(6);
    // Wait briefly to ensure DMA starts properly
    while(!LL_DMA_IsActiveFlag_TC2(DMA1));

    // Clear STOP flag to avoid any unwanted stoppage
    LL_I2C_ClearFlag_STOP(I2Cx);

    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_2);

    // Clear STOP flag at the end of the transfer
    LL_I2C_ClearFlag_STOP(I2Cx);
}
/*
 * Test Code DMA interrupt
 * */
void i2cdevReadReg16_DMA_IT(I2C_TypeDef *I2Cx ,
                             uint8_t SlaveAddr_IC ,
                             uint16_t target_reg,
                             uint8_t *p_values,
                             uint32_t size){

    // Address process
    SlaveAddr_IC = SlaveAddr_IC << 1;

    // Start DMA transfer
    LL_DMA_ConfigAddresses(DMA1,
                           LL_DMA_CHANNEL_7,
                           LL_I2C_DMA_GetRegAddr(I2Cx, LL_I2C_DMA_REG_DATA_RECEIVE),
                           (uint32_t)p_values,
                           LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_7, size);

    // Enable I2C DMA request for RX
    LL_I2C_EnableDMAReq_RX(I2Cx);

    // Handle I2C transfer to send register address
    LL_I2C_HandleTransfer(I2Cx , SlaveAddr_IC , LL_I2C_ADDRSLAVE_7BIT , 2 , LL_I2C_MODE_SOFTEND , LL_I2C_GENERATE_START_WRITE);

    // Send register address high byte
    LL_I2C_TransmitData8(I2Cx, (uint8_t)(target_reg >> 8));
    // No need to wait for TXE flag, as DMA will handle the transfer

    // Send register address low byte
    LL_I2C_TransmitData8(I2Cx, (uint8_t)(target_reg & 0xFF));
    // No need to wait for TXE flag, as DMA will handle the transfer

    // Handle I2C transfer to receive data
    LL_I2C_HandleTransfer(I2Cx, SlaveAddr_IC, LL_I2C_ADDRSLAVE_7BIT, size , LL_I2C_MODE_AUTOEND , LL_I2C_GENERATE_START_READ);

    // Enable DMA channel
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_7);

    // Enable DMA transfer complete interrupt
    LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_7);
}

//I2C DUAL BUFFER TEST
void I2C_Transmit(	I2C_TypeDef *I2Cx,
					uint8_t SlaveAddr_IC,
					uint16_t target_reg,
					uint8_t *p_values,
					uint32_t Length) {
	uint32_t remaining = Length;
	uint8_t *pBuffer = p_values;
	uint8_t *currentBuffer = buffer1;
	uint8_t *nextBuffer = buffer2;

	SlaveAddr_IC = SlaveAddr_IC << 1;
	while (LL_I2C_IsActiveFlag_BUSY(I2Cx));
	LL_I2C_HandleTransfer(I2Cx , SlaveAddr_IC , LL_I2C_ADDRSLAVE_7BIT , 2 , LL_I2C_MODE_AUTOEND , LL_I2C_GENERATE_START_WRITE);
	LL_I2C_TransmitData8(I2Cx, (uint8_t)(target_reg >> 8));
	while (!LL_I2C_IsActiveFlag_TXE(I2Cx));
	LL_I2C_TransmitData8(I2Cx, (uint8_t)(target_reg & 0xFF));
	while (!LL_I2C_IsActiveFlag_TXE(I2Cx));

	// 第一次填充currentBuffer
	uint32_t chunk_size = (remaining > BUFFER_SIZE) ? BUFFER_SIZE : remaining;
    memcpy(currentBuffer, pBuffer, chunk_size);
    pBuffer += chunk_size;
    remaining -= chunk_size;
    // 启动第一次DMA传输
    dma_transfer_complete = 0;
    LL_I2C_HandleTransfer(I2Cx, SlaveAddr_IC, LL_I2C_ADDRSLAVE_7BIT, chunk_size, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_WRITE);
    LL_DMA_ConfigAddresses(	DMA1,
    						LL_DMA_CHANNEL_6,
							(uint32_t)currentBuffer,
							LL_I2C_DMA_GetRegAddr(I2Cx, LL_I2C_DMA_REG_DATA_TRANSMIT),
	                        LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
	LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_6, chunk_size);
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_6);
    LL_I2C_EnableDMAReq_TX(I2Cx);

    while (remaining > 0) {
        // 等待当前传输完成
        while (!dma_transfer_complete);
        dma_transfer_complete = 0;
        // 填充nextBuffer
        chunk_size = (remaining > BUFFER_SIZE) ? BUFFER_SIZE : remaining;
        memcpy(nextBuffer, pBuffer, chunk_size);
        pBuffer += chunk_size;
        remaining -= chunk_size;
	    // 切换缓冲区
        currentBuffer = nextBuffer;
        nextBuffer = (currentBuffer == buffer1) ? buffer2 : buffer1;
        // 启动下一次DMA传输
        LL_I2C_HandleTransfer(I2Cx, SlaveAddr_IC, LL_I2C_ADDRSLAVE_7BIT, chunk_size, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_WRITE);
        LL_DMA_ConfigAddresses(DMA1, LL_DMA_CHANNEL_6, (uint32_t)currentBuffer,
                               LL_I2C_DMA_GetRegAddr(I2Cx, LL_I2C_DMA_REG_DATA_TRANSMIT),
                               LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
        LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_6, chunk_size);
        LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_6);
        LL_I2C_EnableDMAReq_TX(I2Cx);
    }

    // 等待最后一次传输完成
    while (!dma_transfer_complete);
}
