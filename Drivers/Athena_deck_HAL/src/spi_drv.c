/*
 * spi_drv.c
 *
 *  Created on: Apr 30, 2024
 *      Author: Yunfan,Hanjie
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "spi_drv.h"


int spi_transmit(SPI_TypeDef* spix, uint8_t data_in){
	int state = 0;
	uint32_t timeout_cnt;
	static const uint32_t timeout_cnt_num = 10000;

	timeout_cnt = 0;

	while(!LL_SPI_IsActiveFlag_TXE(spix)){
		timeout_cnt ++;
		if(timeout_cnt > timeout_cnt_num){
			state = -1;
			break;
		}
	}

	LL_SPI_TransmitData8(spix, data_in);

	while(LL_SPI_IsActiveFlag_BSY(spix)){
		timeout_cnt ++;
		if(timeout_cnt > timeout_cnt_num){
			state = -1;
			break;
		}
	}

	return state;
}

int spi_receive(SPI_TypeDef* spix, uint8_t *data_out){
	int state = 0;
	uint32_t timeout_cnt = 0;
	static const uint32_t timeout_cnt_num = 10000;

	timeout_cnt = 0;
	while(!LL_SPI_IsActiveFlag_RXNE(spix)){
		timeout_cnt ++;
		if(timeout_cnt > timeout_cnt_num){
			state = -1;
			break;
		}
	}
	*data_out = LL_SPI_ReceiveData8(spix);
	return state;
}


int spi_transmit_receive(SPI_TypeDef* spix, uint8_t data_in, uint8_t *data_out){
	int state = 0;
	*data_out = 0;
	uint32_t timeout_cnt;
	static const uint32_t timeout_cnt_num = 10000;

	timeout_cnt = 0;

	while(!LL_SPI_IsActiveFlag_TXE(spix)){
		timeout_cnt ++;
		if(timeout_cnt > timeout_cnt_num){
			state = -1;
			break;
		}
	}
	LL_SPI_TransmitData8(spix, data_in);
	timeout_cnt = 0;
	while(LL_SPI_IsActiveFlag_BSY(spix)){
		timeout_cnt ++;
		if(timeout_cnt > timeout_cnt_num){
			state = -1;
			break;
		}
	}
	timeout_cnt = 0;
	while(LL_SPI_IsActiveFlag_RXNE(spix)&&(LL_SPI_GetRxFIFOLevel(spix)==LL_SPI_RX_FIFO_EMPTY))
	{
		timeout_cnt ++;
		if(timeout_cnt > timeout_cnt_num){
			state = -1;
			break;
		}
	}
	*data_out = LL_SPI_ReceiveData8(spix);
	return state;
}

bool spi_transmit_DMA(SPI_TypeDef* spix, uint8_t data_in, uint8_t size)
{

}

bool spi_receive_DMA(SPI_TypeDef* spix, uint8_t *data_out, uint8_t size)
{

}
