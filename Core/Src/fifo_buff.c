#include "fifo_buff.h"
#include <limits.h>

#define IS_POWER_OF_2(x) ((x) != 0 && (((x) & ((x)-1)) == 0))

static inline uint32_t min_fifo(uint32_t X, uint32_t Y)
{
    return ((X) > (Y) ? (Y) : (X));
}

/**
  * @brief      计算能四舍五入到下一个2次幂的最大数
  * @param[in]  Num: 要更改的数字
  * @retval     返回更改后的数字
  * @attention
  */
uint32_t roundup_pow_of_two(uint32_t Num)
{
    uint32_t result = 1;

    if (IS_POWER_OF_2(Num) || Num == 0)
        return Num;
    else if (Num > LONG_MAX)
        return (LONG_MAX ^ ULONG_MAX); // WARN: 如果Num大于(LONG_MAX+1)，那么result将等于(LONG_MAX+1)

    while (Num)
    {
        Num >>= 1;
        result <<= 1;
    }

    return result;
}

/**
  * @brief          计算能四舍五入到下一个2次幂的最小数
  * @param[in]      Num: 要更改的数字
  * @retval         返回更改后的数字
  * @attention
  */
uint32_t rounddown_pow_of_two(uint32_t Num)
{
    uint32_t result = 1;

    if (IS_POWER_OF_2(Num) || Num == 0)
        return Num;
    else if (Num > LONG_MAX)
        return (LONG_MAX ^ ULONG_MAX); // WARN: 如果Num大于(LONG_MAX+1)，那么result将等于(LONG_MAX+1)

    while (Num)
    {
        Num >>= 1;
        result <<= 1;
    }

    return result >> 1;
}

/**
  * @brief          环形缓冲区的初始化
  * @param[in]      pfifo: 将循环缓冲区初始化
  * @param[in]      buff: 用于存储数据的环形缓冲区
  * @param[in]      size: 缓冲区的大小
  * @retval         成功返回0
  * @attention
  */
int32_t fifo_init(fifo_rx_def *pfifo, uint8_t *buff, uint32_t size)
{
    assert(pfifo != NULL || buff != NULL);

    if (!IS_POWER_OF_2(size)) // 必须 2 的幂次方
    {
        return -1;
    }

    pfifo->in = 0;
    pfifo->out = 0;
    pfifo->buffer = buff;
    pfifo->size = size; // 必须最后设置大小
    pfifo->last_cnt = size;

    return 0;
}

/**
  * @brief          从环形缓冲区获取数据
  * @param[in]      pfifo: 存储数据的环形缓冲区
  * @param[in]      buffer: 将存储来自循环缓冲区的数据的目标缓冲区
  * @param[in]      len: 要从循环缓冲区中获取的长度
  * @retval         从循环缓冲区获得的实际长度
  * @attention
  */
uint32_t fifo_read_buff(fifo_rx_def *pfifo, uint8_t *buffer, uint32_t len)
{
    uint32_t length;

    len = min_fifo(len, pfifo->in - pfifo->out); //获取队列out开始到数组结束的大小

    /* first get the data from pfifo->out until the end of the buffer */
    length = min_fifo(len, pfifo->size - (pfifo->out & (pfifo->size - 1))); //获取队列out开始到数组结束的大小
    memcpy(buffer, pfifo->buffer + (pfifo->out & (pfifo->size - 1)), length);

    /* then get the rest (if any) from the beginning of the buffer */
    memcpy(buffer + length, pfifo->buffer, len - length);

    pfifo->out += len;

    return len;
}

/**
  * @brief          将数据放入环形缓冲区
  * @param[in]      pfifo: 存储数据的环形缓冲区
  * @param[in]      buffer: 要存储到环形缓冲区中的数据
  * @param[in]      len: 要存储到环形缓冲区中的数据长度
  * @retval         存储在环形缓冲区中的实际大小
  * @attention
  */
uint32_t fifo_write_buff(fifo_rx_def *pfifo, uint8_t *buffer, uint32_t len)
{
    uint32_t length;

    len = min_fifo(len, (pfifo->size - (pfifo->in - pfifo->out)));

    length = min_fifo(len, pfifo->size - (pfifo->in & (pfifo->size - 1)));

    memcpy(pfifo->buffer + (pfifo->in & pfifo->size - 1), buffer, length);
    memcpy(pfifo->buffer, buffer + length, len - length);

    pfifo->in += len;

    return len;
}

/**
  * @brief          获取环形缓冲区的可用内存大小
  * @param[in]      pfifo: 存储数据的环形缓冲区
  * @retval         环形缓冲区的可用内存大小
  * @attention
  */
unsigned int fifo_get_free(fifo_rx_def *pfifo)
{
    return ((pfifo->size > 0) ? (pfifo->size - (pfifo->in - pfifo->out)) : 0);
}

/**
  * @brief          获取环形缓冲区已使用的内存大小
  * @param[in]      pfifo: 存储数据的环形缓冲区
  * @retval         环形缓冲区已使用的内存大小
  * @attention
  */
unsigned int fifo_get_full(fifo_rx_def *pfifo)
{
    return (pfifo->in - pfifo->out);
}

/**
  * @brief          检查环形缓冲区是否为空
  * @param[in]      pfifo: 存储数据的环形缓冲区
  * @retval         如果没有数据就返回1
  * @attention
  */
unsigned int fifo_is_empty(fifo_rx_def *pfifo)
{
    return ((pfifo->size > 0) && (pfifo->in == pfifo->out));
}

/**
  * @brief          检查环形缓冲区是否已满
  * @param[in]      pfifo: 存储数据的环形缓冲区
  * @retval         如果满了就返回1
  * @attention
  */
unsigned int fifo_is_full(fifo_rx_def *pfifo)
{
    return ((pfifo->size == 0) || (pfifo->size == (pfifo->in - pfifo->out)));
}


