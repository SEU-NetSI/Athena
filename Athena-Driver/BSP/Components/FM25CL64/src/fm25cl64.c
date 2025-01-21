/*
 * fm25cl64.c
 *
 *  Created on: Nov 23, 2024
 *      Author: Hanjie Yu
 */

#include "fm25cl64.h"
#include "fm25_platform.h"
#include <stdio.h>

static uint8_t data[520];

/*缺省片选处理函数*/
static void FM25ChipSelectDefault(FM25CSType cs);

/*从FM25XXX读取单个字节,从随机地址读取*/
uint8_t ReadByteFromFM25xxx(FM25ObjectType *fram, uint32_t regAddress) {
	uint8_t rData;

	ReadBytesFromFM25xxx(fram, regAddress, &rData, 1);

	return rData;
}

/*向FM25XXX写入单个字节*/
void WriteByteToFM25xxx(FM25ObjectType *fram, uint32_t regAddress, uint8_t data) {

	WriteBytesToFM25xxx(fram, regAddress, &data, 1);
}

/* 设置写使能所存器*/
void SetWriteEnableLatchForFM25xxx(FM25ObjectType *fram) {
	uint8_t opCode = FM25_WREN;

	fram->ChipSelect(FM25CS_Enable);
	fram->Write(&opCode, 1);
	fram->ChipSelect(FM25CS_Disable);

	ReadStatusForFM25xxx(fram);
}

/* 设置写使能所存器*/
void SetWriteEnableForFM25xxx(FM25ObjectType *fram) {
	uint8_t opCode = FM25_WREN;
	fram->ChipSelect(FM25CS_Enable);
	fram->Write(&opCode, 1);
	fram->ChipSelect(FM25CS_Disable);
}

/* 复位写使能所存器*/
void ResetWriteEnableLatchForFM25xxx(FM25ObjectType *fram) {
	uint8_t opCode = FM25_WRDI;

	fram->ChipSelect(FM25CS_Enable);
	fram->Write(&opCode, 1);
	fram->ChipSelect(FM25CS_Disable);

	ReadStatusForFM25xxx(fram);
}

/*读FM25xxx状态寄存器*/
void ReadStatusForFM25xxx(FM25ObjectType *fram) {
	uint8_t opCode = FM25_RDSR;
	uint8_t status;

	fram->ChipSelect(FM25CS_Enable);
	fram->Write(&opCode, 1);
	fram->Delayms(1);
	fram->Read(&status, 1);
	fram->ChipSelect(FM25CS_Disable);

	fram->status = status;
}

/*写FM25xxx状态寄存器*/
void WriteStatusForFM25xx(FM25ObjectType *fram, uint8_t cmd) {
	uint8_t data[2];

	data[0] = FM25_WRSR;
	data[1] = cmd;

	if (((fram->status) & 0x02) != 0x02) {
		SetWriteEnableLatchForFM25xxx(fram);
	}

	fram->ChipSelect(FM25CS_Enable);
	fram->Write(data, 2);
	fram->ChipSelect(FM25CS_Disable);

	ReadStatusForFM25xxx(fram);

}

/*从FM25xxx读取数据*/
void ReadBytesFromFM25xxx(FM25ObjectType *fram, uint32_t regAddress,
		uint8_t *rData, uint16_t rSize) {
	uint8_t data[4];
	uint16_t index = 0;
	uint8_t temp;

	data[index++] = FM25_READ;

	if (fram->memAddLength == FM258BitMemAdd) {
		data[index++] = (uint8_t) regAddress;

		if ((fram->mode == FM25L04B) || (fram->mode == FM25040B)) {
			temp = (uint8_t) (regAddress >> 8);
			data[0] |= ((temp & 0x01) << 3);
		}
	} else if (fram->memAddLength == FM2516BitMemAdd) {
		data[index++] = (uint8_t) (regAddress >> 8);
		data[index++] = (uint8_t) regAddress;
	} else {
		data[index++] = (uint8_t) (regAddress >> 16);
		data[index++] = (uint8_t) (regAddress >> 8);
		data[index++] = (uint8_t) regAddress;
	}

	fram->ChipSelect(FM25CS_Enable);
	fram->Write(data, index);
	fram->Delayms(1);
	fram->Read(rData, rSize);
	fram->ChipSelect(FM25CS_Disable);
}

/*向FM25xxx写入数据*/
void WriteBytesToFM25xxx(FM25ObjectType *fram, uint32_t regAddress,
		uint8_t *wData, uint16_t wSize) {
	uint8_t temp;
	uint16_t index = 0;

	data[index++] = FM25_WRITE;

	if (fram->memAddLength == FM258BitMemAdd) {
		data[index++] = (uint8_t) regAddress;

	if ((fram->mode == FM25L04B) || (fram->mode == FM25040B)) {
		temp = (uint8_t) (regAddress >> 8);
		data[0] |= ((temp & 0x01) << 3);
	}
	} else if (fram->memAddLength == FM2516BitMemAdd) {
		data[index++] = (uint8_t) (regAddress >> 8);
		data[index++] = (uint8_t) regAddress;
	} else {
		data[index++] = (uint8_t) (regAddress >> 16);
		data[index++] = (uint8_t) (regAddress >> 8);
		data[index++] = (uint8_t) regAddress;
	}

	memcpy(&data[index], wData, wSize);
	index += wSize;

	if (((fram->status) & 0x02) != 0x02) {
		SetWriteEnableLatchForFM25xxx(fram);
	}
	SetWriteEnableForFM25xxx(fram);
	fram->ChipSelect(FM25CS_Enable);
	fram->Write(data, index);
	fram->ChipSelect(FM25CS_Disable);
}

/*FM25对象初始化*/
void Fm25cxxInitialization(FM25ObjectType *fram,        //FM25xxx对象实体
		FM25ModeType mode,          //设备类型
		Fm25Read read,              //读FM25xxx对象操作指针
		Fm25Write write,            //写FM25xxx对象操作指针
		Fm25Delayms delayms,        //延时操作指针
		Fm25ChipSelect cs          //片选信号函数指针
		) {
	if ((fram == NULL) || (read == NULL) || (write == NULL)
			|| (delayms == NULL)) {
		return;
	}
	fram->Read = read;
	fram->Write = write;
	fram->Delayms = delayms;

	if (cs != NULL) {
		fram->ChipSelect = cs;
	} else {
		fram->ChipSelect = FM25ChipSelectDefault;
	}

	if (mode >= FM25Number) {
		return;
	}
	fram->mode = mode;

	if (mode < FM25C160B) {
		fram->memAddLength = FM258BitMemAdd;
	} else if (mode < FM25V10) {
		fram->memAddLength = FM2516BitMemAdd;
	} else {
		fram->memAddLength = FM2524BitMemAdd;
	}

	//写允许
	SetWriteEnableLatchForFM25xxx(fram);
	LL_mDelay(3);

}

/*缺省片选处理函数*/
static void FM25ChipSelectDefault(FM25CSType cs) {
	//用于在SPI通讯时，片选信号硬件电路选中的情况
	return;
}
