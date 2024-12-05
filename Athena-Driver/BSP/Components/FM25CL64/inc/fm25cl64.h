/*
 * fm25cl64.h
 *
 *  Created on: Oct 10, 2024
 *      Author: Hanjie Yu
 */

#ifndef BSP_COMPONENTS_FM25CL64_INC_FM25CL64_H_
#define BSP_COMPONENTS_FM25CL64_INC_FM25CL64_H_

#include "stm32h7xx_ll_gpio.h"
#include "main.h"

/* 定义操作码 */
#define FM25_WREN       0x06	//写使能
#define FM25_WRDI       0x04	//写失能
#define FM25_RDSR       0x05	//读状态寄存器
#define FM25_WRSR       0x01	//写状态寄存器
#define FM25_READ       0x03	//读存储数据
#define FM25_WRITE      0x02	//写存储数据

#define FM25_WPEN       0x80	//写保护使能
#define FM25_BPNONE     0x00	//无保护区域
#define FM25_BPQUARTER  0x04	//保护四分之一区域
#define FM25_BPHALF     0x08	//保护一半区域
#define FM25_BPALL      0x0C	//保护全部区域

/*定义FM25XXX类型枚举*/
typedef enum FM25Mode {
  FM25L04B,
  FM25040B,
  FM25C160B,
  FM25L16B,
  FM25CL64B,
  FM25640B,
  FM25V01A,
  FM25W256,
  FM25V02A,
  FM25V05,
  FM25V10,
  FM25Number
}FM25ModeType;

/*定义存储器寄存器地址字节枚举*/
typedef enum FM25MemAddLength {
  FM258BitMemAdd,
  FM2516BitMemAdd,
  FM2524BitMemAdd
}FM25MemAddLengthType;

/* 定义写保护操作枚举 */
typedef enum FM25WP {
  FM25WP_Enable,
  FM25WP_Disable
}FM25WPType;

/* 定义片选信号枚举 */
typedef enum FM25CS {
  FM25CS_Enable,
  FM25CS_Disable
}FM25CSType;

/* 定义片选信号枚举 */
typedef enum FM25Hold {
  FM25Hold_Enable,
  FM25Hold_Disable
}FM25HoldType;

/* 定义FM25C对象类型 */
typedef struct FM25Object {
  uint8_t status;       //状态寄存器
  FM25ModeType mode;    //设备类型
  FM25MemAddLengthType memAddLength;    //寄存器地址长度
  void (*Read)(uint8_t *rData,uint16_t rSize);  //读数据操作指针
  void (*Write)(uint8_t *wData,uint16_t wSize); //写数据操作指针
  void (*ChipSelect)(FM25CSType cs);    //片选信号
  void (*Delayms)(volatile uint32_t nTime);     //延时操作指针
}FM25ObjectType;

/* 定义读数据操作函数指针类型 */
typedef void (*Fm25Read)(uint8_t *rData,uint16_t rSize);
/* 定义写数据操作函数指针类型 */
typedef void (*Fm25Write)(uint8_t *wData,uint16_t wSize);
/* 定义延时操作函数指针类型 */
typedef void (*Fm25Delayms)(volatile uint32_t nTime);
/* 定义片选操作函数指针类型 */
typedef void (*Fm25ChipSelect)(FM25CSType cs);


/*FM24C对象初始化*/
void Fm25cxxInitialization(FM25ObjectType *fram,        //FM25xxx对象实体
                           FM25ModeType mode,           //设备类型
                           Fm25Read read,               //读FM25xxx对象操作指针
                           Fm25Write write,             //写FM25xxx对象操作指针
                           Fm25Delayms delayms,         //延时操作指针
                           Fm25ChipSelect cs           //片选信号函数指针
                          );

/* 设置写使能所存器*/
void SetWriteEnableLatchForFM25xxx(FM25ObjectType *fram);

/* 复位写使能所存器*/
void ResetWriteEnableLatchForFM25xxx(FM25ObjectType *fram);

/*读FM25xxx状态寄存器*/
void ReadStatusForFM25xxx(FM25ObjectType *fram);

/*写FM25xxx状态寄存器*/
void WriteStatusForFM25xx(FM25ObjectType *fram,uint8_t cmd);

/*从FM25XXX读取单个字节,从随机地址读取*/
uint8_t ReadByteFromFM25xxx(FM25ObjectType *fram,uint32_t regAddress);

/*向FM25XXX写入单个字节*/
void WriteByteToFM25xxx(FM25ObjectType *fram,uint32_t regAddress,uint8_t data);

/*从FM25xxx读取数据*/
void ReadBytesFromFM25xxx(FM25ObjectType *fram,uint32_t regAddress,uint8_t *rData,uint16_t rSize);

/*向FM25xxx写入数据*/
void WriteBytesToFM25xxx(FM25ObjectType *fram,uint32_t regAddress,uint8_t *wData,uint16_t wSize);

#endif /* BSP_COMPONENTS_FM25CL64_INC_FM25CL64_H_ */
