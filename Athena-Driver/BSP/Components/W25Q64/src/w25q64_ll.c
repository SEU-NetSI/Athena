/*
 * w25q64_ll.c
 *
 *  Created on: May 20, 2024
 *      Author: Yunfan Li, Hanjie Yu
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "w25q64_ll.h"
#include "spi_drv.h"

#define SPI_BUFFER_MAX_SIZE 240
static uint8_t spiTxBuffer[SPI_BUFFER_MAX_SIZE];
static uint8_t spiRxBuffer[SPI_BUFFER_MAX_SIZE];

void W25Qx_Enable_DMA()
{
	spiBeginTransaction();
	LL_GPIO_ResetOutputPin(W25QXX_CS_GPIO_Port, W25QXX_CS_Pin);
}

void W25Qx_Disable_DMA()
{
	LL_GPIO_SetOutputPin(W25QXX_CS_GPIO_Port, W25QXX_CS_Pin);
	spiEndTransaction();
}

void spiRead(const void* cmd,
			size_t cmdLength,
			void *data,
			size_t dataLength,
			SPI_TypeDef* SPIx)
{
	spiBeginTransaction();
	W25Qx_Enable();
	memcpy(spiTxBuffer, cmd, cmdLength);
	memset(spiTxBuffer + cmdLength, DUMMY_BYTE, dataLength);
	spiExchange(SPIx, cmdLength + dataLength, spiTxBuffer, spiRxBuffer);
	memcpy(data, spiRxBuffer + cmdLength, dataLength);
	W25Qx_Disable();
	spiEndTransaction();
}

uint8_t BSP_W25Qx_Init(void)
{
	BSP_W25Qx_Reset();
	return BSP_W25Qx_GetStatus();
}


static void	BSP_W25Qx_Reset(void)
{
	uint8_t cmd[2]= {RESET_ENABLE_CMD,RESET_MEMORY_CMD};
	uint8_t dummy[]= {DUMMY_BYTE};
	spiRead(cmd, 2, dummy, 1, SPI3);
	vTaskDelay(1);
}


static uint8_t BSP_W25Qx_GetStatus(void)
{
	uint8_t cmd[] = {READ_STATUS_REG1_CMD};
	uint8_t status= 0xFF;
	spiRead(cmd, 1, &status, 1, SPI3);

	if((status & W25Q128FV_FSR_BUSY) != 0)
	{
		return W25Qx_BUSY;
	}
	else
	{
		return W25Qx_OK;
	}
}

/**********************************************************************************
 * 函数功能: 获取设备ID
 */
void BSP_W25Qx_Read_ID(uint8_t *ID)
{
	uint8_t cmd[4] = {READ_ID_CMD, DUMMY_BYTE, DUMMY_BYTE, 0x00};
	spiRead(cmd, 4, ID, 2, SPI3);
}

/**********************************************************************************
 * 函数功能: 写使能
 */
uint8_t BSP_W25Qx_WriteEnable(void)
{
	uint8_t cmd[] = {WRITE_ENABLE_CMD};
//	uint32_t tickstart = HAL_GetTick();

	/*Select the FLASH: Chip Select low */
	W25Qx_Enable_DMA();
	/* Send the read ID command */
	uint8_t tmpdummy[1]={0x00};
	spiExchange(SPI3, 1, cmd, tmpdummy);
	/*Deselect the FLASH: Chip Select high */
	W25Qx_Disable_DMA();

	/* Wait the end of Flash writing */
//	while(BSP_W25Qx_GetStatus() == W25Qx_BUSY);
//	{
//		/* Check for the Timeout */
//		if((HAL_GetTick() - tickstart) > W25Qx_TIMEOUT_VALUE)
//		{
//				return W25Qx_TIMEOUT;
//		}
//	}
	return W25Qx_OK;
}

/**********************************************************************************
 * 函数功能: 读数据
 * 输入参数: 缓存数组指针、读地址、字节数
 */
uint8_t BSP_W25Qx_Read(uint8_t* pData, uint32_t ReadAddr, uint32_t Size)
{
	uint8_t cmd[4];

	/* Configure the command */
	cmd[0] = READ_CMD;
	cmd[1] = (uint8_t)(ReadAddr >> 16);
	cmd[2] = (uint8_t)(ReadAddr >> 8);
	cmd[3] = (uint8_t)(ReadAddr);

	W25Qx_Enable();
	/* Send the read ID command */
	/* Reception of the data */
	spiRead(cmd, 4, pData, Size, SPI3);
//	if (LL_SPI_Receive(SPI1, pData,Size,W25Qx_TIMEOUT_VALUE) != LL_OK)
//   {
//		return W25Qx_ERROR;
//   }
	W25Qx_Disable();
	return W25Qx_OK;
}

/**********************************************************************************
  * 函数功能: 写数据
  * 输入参数: 缓存数组指针、写地址、字节数
  */
uint8_t BSP_W25Qx_Write(uint8_t* pData, uint32_t WriteAddr, uint32_t Size)
{
	uint8_t cmd[4];
	uint32_t end_addr, current_size, current_addr;
//	uint32_t tickstart = HAL_GetTick();

	/* Calculation of the size between the write address and the end of the page */
  current_addr = 0;

  while (current_addr <= WriteAddr)
  {
    current_addr += W25Q128FV_PAGE_SIZE;
  }
  current_size = current_addr - WriteAddr;

  /* Check if the size of the data is less than the remaining place in the page */
  if (current_size > Size)
  {
    current_size = Size;
  }

  /* Initialize the adress variables */
  current_addr = WriteAddr;
  end_addr = WriteAddr + Size;

  /* Perform the write page by page */
  do
  {
	/* Configure the command */
	cmd[0] = PAGE_PROG_CMD;
	cmd[1] = (uint8_t)(current_addr >> 16);
	cmd[2] = (uint8_t)(current_addr >> 8);
	cmd[3] = (uint8_t)(current_addr);

	/* Enable write operations */
	BSP_W25Qx_WriteEnable();

	W25Qx_Enable_DMA();
    /* Send the command */
    if (!spiExchange(SPI3, 4, cmd, spiRxBuffer))
    {
      return W25Qx_ERROR;
    }

    /* Transmission of the data */
    if (!spiExchange(SPI3, current_size, pData, spiRxBuffer))
    {
      return W25Qx_ERROR;
    }
	W25Qx_Disable_DMA();
    /* Wait the end of Flash writing */
	while(BSP_W25Qx_GetStatus() == W25Qx_BUSY);
//	{
//		/* Check for the Timeout */
//		if((HAL_GetTick() - tickstart) > W25Qx_TIMEOUT_VALUE)
//		{
//			return W25Qx_TIMEOUT;
//		}
//	}

    /* Update the address and size variables for next page programming */
    current_addr += current_size;
    pData += current_size;
    current_size = ((current_addr + W25Q128FV_PAGE_SIZE) > end_addr) ? (end_addr - current_addr) : W25Q128FV_PAGE_SIZE;
  } while (current_addr < end_addr);

	return W25Qx_OK;
}

/**********************************************************************************
  * 函数功能: 扇区擦除
  * 输入参数: 地址
  */
uint8_t BSP_W25Qx_Erase_Block(uint32_t Address)
{
	uint8_t cmd[4];
//	uint32_t tickstart = HAL_GetTick();
	cmd[0] = SECTOR_ERASE_CMD;
	cmd[1] = (uint8_t)(Address >> 16);
	cmd[2] = (uint8_t)(Address >> 8);
	cmd[3] = (uint8_t)(Address);

	/* Enable write operations */
	BSP_W25Qx_WriteEnable();

	/*Select the FLASH: Chip Select low */
	W25Qx_Enable_DMA();
	/* Send the read ID command */
	spiExchange(SPI3, 4, cmd, spiRxBuffer);
	/*Deselect the FLASH: Chip Select high */
	W25Qx_Disable_DMA();

	/* Wait the end of Flash writing */
	while(BSP_W25Qx_GetStatus() == W25Qx_BUSY);
//	{
//		/* Check for the Timeout */
//    if((HAL_GetTick() - tickstart) > W25Q128FV_SECTOR_ERASE_MAX_TIME)
//    {
//			return W25Qx_TIMEOUT;
//    }
//	}
	return W25Qx_OK;
}

 /**********************************************************************************
  * 函数功能: 芯片擦除
  */
uint8_t BSP_W25Qx_Erase_Chip(void)
{
	uint8_t cmd[4];
//	uint32_t tickstart = HAL_GetTick();
	cmd[0] = CHIP_ERASE_CMD;

	/* Enable write operations */
	BSP_W25Qx_WriteEnable();

	/*Select the FLASH: Chip Select low */
	W25Qx_Enable_DMA();
	/* Send the read ID command */
	spiExchange(SPI3, 1, cmd, spiRxBuffer);
	/*Deselect the FLASH: Chip Select high */
	W25Qx_Disable_DMA();

	/* Wait the end of Flash writing */
	while(BSP_W25Qx_GetStatus() == W25Qx_BUSY);
//	{
//		/* Check for the Timeout */
//    if((HAL_GetTick() - tickstart) > W25Q128FV_BULK_ERASE_MAX_TIME)
//    {
//			return W25Qx_TIMEOUT;
//    }
//	}
	return W25Qx_OK;
}
