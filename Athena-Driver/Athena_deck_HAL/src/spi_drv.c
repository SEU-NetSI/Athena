/*
 * spi_drv.c
 *
 *  Created on: Apr 30, 2024
 *      Author: Yunfan,Hanjie
 */

#include <stdio.h>
#include <stdint.h>
#include "spi_drv.h"
#include "main.h"
#include "stm32h7xx_ll_dma.h"
#include "stm32h7xx_ll_spi.h"
#include "stm32h7xx_ll_bus.h"
#include "stm32h7xx_ll_gpio.h"

#define SPI_TIMEOUT_MS         5      // 单次传输超时时间（ms）
#define ERROR_THRESHOLD        3      // 错误计数阈值
#define MAX_RETRIES            5      // 最大重试次数
#define ERROR_GPIO_PIN         LL_GPIO_PIN_9
#define ERROR_GPIO_PORT        GPIOE

bool spiDeckExchange(size_t length, const uint8_t* data_tx, uint8_t * data_rx)
{
	LL_DMA_ConfigAddresses(DMA1, LL_DMA_STREAM_1, (uint32_t)data_tx, LL_SPI_DMA_GetTxRegAddr(SPI2), LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
	LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_1, length);
	LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_1);
	LL_SPI_EnableDMAReq_TX(SPI2);

	LL_DMA_ConfigAddresses(DMA1, LL_DMA_STREAM_0, LL_SPI_DMA_GetRxRegAddr(SPI2), (uint32_t)data_rx, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
	LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_0, length);
	LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_0);
	LL_SPI_EnableDMAReq_RX(SPI2);
	LL_SPI_Enable(SPI2);
	LL_SPI_StartMasterTransfer(SPI2);
    // Wait for completion
    bool result = (xSemaphoreTake(txComplete, portMAX_DELAY) == pdTRUE)
             && (xSemaphoreTake(rxComplete, portMAX_DELAY) == pdTRUE);

    return result;
}

// 静态变量用于错误计数（实际应用中建议使用原子操作）
static volatile uint8_t spiErrorCount = 0;

bool spi2Exchange(SPI_TypeDef* SPIx, size_t length,
                const uint8_t* data_tx, uint8_t* data_rx)
{
    uint8_t retries = 0;
    const TickType_t xTicksToWait = pdMS_TO_TICKS(SPI_TIMEOUT_MS);

    while(retries < MAX_RETRIES) {
        // 配置DMA传输
        LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_1);
        LL_DMA_ConfigAddresses(DMA1, LL_DMA_STREAM_1,
                             (uint32_t)data_tx,
                             LL_SPI_DMA_GetTxRegAddr(SPI2),
                             LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
        LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_1, length);
        LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_1);

        LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_0);
        LL_DMA_ConfigAddresses(DMA1, LL_DMA_STREAM_0,
                             LL_SPI_DMA_GetRxRegAddr(SPI2),
                             (uint32_t)data_rx,
                             LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
        LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_0, length);
        LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_0);

        // 使能SPI DMA请求
        LL_SPI_EnableDMAReq_TX(SPI2);
        LL_SPI_EnableDMAReq_RX(SPI2);

        // 启动传输
        LL_SPI_Enable(SPI2);
        LL_SPI_StartMasterTransfer(SPI2);

        // 等待传输完成（带超时）
        bool txDone = (xSemaphoreTake(txComplete, xTicksToWait) == pdTRUE);
        bool rxDone = (xSemaphoreTake(rxComplete, xTicksToWait) == pdTRUE);
        uint8_t test_char = *data_rx;

        if((*data_rx & *(data_rx+1)) != 0xFF){
            // 传输成功，重置错误计数器
            LL_GPIO_ResetOutputPin(ERROR_GPIO_PORT, ERROR_GPIO_PIN);
            return true;
        } else {
        	spiErrorCount ++;
        	LL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
        	osDelay(200);
        	LL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
            // 传输失败处理
            LL_SPI_Disable(SPI2);
            LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_0);
            LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_1);
            // 增加错误计数
            if(++retries >= ERROR_THRESHOLD) {
                LL_GPIO_SetOutputPin(ERROR_GPIO_PORT, ERROR_GPIO_PIN);
                osDelay(200);
                LL_GPIO_ResetOutputPin(ERROR_GPIO_PORT, ERROR_GPIO_PIN);
                printf("Retry over 5 times!\n");
            }
            // 重试前延时
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
    // 超过最大重试次数
    return false;
}

void spiBeginTransaction()
{
	xSemaphoreTake(spiMutex, portMAX_DELAY);

}

void spiEndTransaction()
{
	xSemaphoreGive(spiMutex);
}

//======

void spiDeckBeginTransaction()
{
	xSemaphoreTake(spiDeckMutex, portMAX_DELAY);

}

void spiDeckEndTransaction()
{
	xSemaphoreGive(spiDeckMutex);
}
