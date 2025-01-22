/*
 * fram_sys_xfer.c
 *
 *  Created on: Jan 21, 2025
 *      Author: mfxjx
 */

#include "fram_sys_xfer.h"
#include "debug.h"

#define BUFFER_SIZE 256
#define CHUNK_SIZE 512
//#define QUICK_XFER
#define DEBUG_PRINT

static size_t Startaddr = 0x0000;
static size_t Endaddr = 0x0FFF;
SemaphoreHandle_t FRAMxferMutex = NULL;
FM25ObjectType fm25;

void Framinit(){
	FRAMxferMutex = xSemaphoreCreateMutex();
	FramArbitrationInit();
	EnableTmux();
	EnableChannelA();
	Fm25cxxInitialization(&fm25,
						  FM25CL64B,
					      ReadDataFromFM25,
					      WriteDataToFM25,
					      LL_mDelay,
					      ChipSelectForFM25
					      );
	EnableChannelB();
}

void FramXfertoPerform(uint8_t* data, size_t len) {
    EnableChannelA();
    if(Startaddr + len > Endaddr){
#ifdef DEBUG_PRINT
    DEBUG_PRINTF("FRAM limit!\n");
#endif
    Startaddr = 0;
    }
    WriteBytesToFM25xxx(&fm25, Startaddr, data, len);
#ifdef DEBUG_PRINT
    DEBUG_PRINTF("Write %d bytes to address %d\n", len, Startaddr);
#endif
    Startaddr += len;
    EnableChannelB();
}


void XfertoPerformance(DataPacket* packet) {
    xSemaphoreTake(FRAMxferMutex, portMAX_DELAY);
    static uint8_t buffer[BUFFER_SIZE];  // 用于暂存数据
    static size_t bufferLen = 0;         // 当前buffer的实际数据长度
    static uint8_t count = 0;            // 用于记录连续放入数据的次数
    static uint8_t lowDataCount = 0;     // 用于记录连续几次数据量小于128B的次数

    // 每次数据包到达时处理
    size_t totalLen = bufferLen + packet->length;

#ifdef QUICK_XFER
    // 如果启用了快速发送模式，并且连续5次数据包小于128B，则直接发送数据
    if (packet->length < 128) {
        lowDataCount++;
        if (lowDataCount >= 5) {
            FramXfertoPerform(buffer, bufferLen + packet->length); // 发送所有缓存数据
            bufferLen = 0;  // 清空缓冲区
            lowDataCount = 0; // 重置计数器
            xSemaphoreGive(FRAMxferMutex);
            return;
        }
    } else {
        lowDataCount = 0; // 重置计数器，如果遇到大于128B的包
    }
#endif

    // 如果数据包小于128B，直接加入缓冲区
    if (packet->length < 128) {
        if (bufferLen + packet->length <= BUFFER_SIZE) {
            memcpy(buffer + bufferLen, packet->content, packet->length);
            bufferLen += packet->length;
            count++;

            // 如果数据包累计长度超过128B时，立即发送数据
            if (bufferLen >= 128) {
                FramXfertoPerform(buffer, bufferLen);  // 直接发送
                bufferLen = 0;  // 清空buffer
                count = 0;   // 重置累计次数
            }
        }
    } else {
        // 数据包大于128B时
        if (bufferLen > 0) {
            // 如果buffer中有数据，拼接当前数据并发送
            uint8_t* combinedBuffer = (uint8_t*)malloc(totalLen);
            memcpy(combinedBuffer, buffer, bufferLen);
            memcpy(combinedBuffer + bufferLen, packet->content, packet->length);

            uint8_t* originalBuffer = combinedBuffer;  // 保存原始指针

            // 分片发送拼接后的数据
            while (totalLen > CHUNK_SIZE) {
                FramXfertoPerform(combinedBuffer, CHUNK_SIZE);
                combinedBuffer += CHUNK_SIZE;
                totalLen -= CHUNK_SIZE;
            }
            if (totalLen > 0) {
                FramXfertoPerform(combinedBuffer, totalLen);
            }

            // 释放原始指针
            free(originalBuffer);

            // 清空buffer
            bufferLen = 0;
        } else {
            // buffer为空，直接发送当前数据
            size_t remainingLen = packet->length;
            uint8_t* currentData = packet->content;
            while (remainingLen > CHUNK_SIZE) {
                FramXfertoPerform(currentData, CHUNK_SIZE);
                currentData += CHUNK_SIZE;
                remainingLen -= CHUNK_SIZE;
            }
            if (remainingLen > 0) {
                FramXfertoPerform(currentData, remainingLen);
            }
        }
    }
    xSemaphoreGive(FRAMxferMutex);
}
