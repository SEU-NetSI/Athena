/*
 * fifo_buff.h
 *
 *  Created on: Feb 21, 2025
 *      Author: mfxjx
 */

#ifndef SYSDRIVER_INC_FIFO_BUFF_H_
#define SYSDRIVER_INC_FIFO_BUFF_H_

#include <string.h>
#include <stdint.h>

#define FIFO_DMA_ERROR_RX_NOT_IDLE (0x1 << 0)   // 非空闲中断
#define FIFO_DMA_ERROR_RX_POINT_NULL (0x1 << 1) // 指针为空
#define FIFO_DMA_ERROR_RX_FULL (0x1 << 2)       // 非空闲中断

typedef struct
{
    uint8_t *buffer;
    uint32_t in;
    uint32_t out;
    uint16_t size;
    uint16_t error; // 接收错误
    uint16_t last_cnt;
} fifo_rx_def;



int32_t fifo_init(fifo_rx_def *pfifo, uint8_t *buff, uint32_t size);
uint32_t fifo_read_buff(fifo_rx_def *pfifo, uint8_t *buffer, uint32_t len);
uint32_t fifo_write_buff(fifo_rx_def *pfifo, uint8_t *buffer, uint32_t len);
unsigned int fifo_get_free(fifo_rx_def *pfifo);
unsigned int fifo_get_full(fifo_rx_def *pfifo);
unsigned int fifo_is_empty(fifo_rx_def *pfifo);
unsigned int fifo_is_full(fifo_rx_def *pfifo);


#endif /* SYSDRIVER_INC_FIFO_BUFF_H_ */
