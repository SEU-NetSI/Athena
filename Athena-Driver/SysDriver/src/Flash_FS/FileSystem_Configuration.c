/*
 * FileSystem_Operational_Wrapper.c
 *
 *  Created on: Nov 13, 2024
 *      Author: 15845
 */

#include <stddef.h>
#include "FileSystem_Configuration.h"

//对w25q64的读函数进行封装，与ringfs文件系统进行链接
ssize_t ringfs_flash_read_wrapper(struct ringfs_flash_partition *flash, int address, void *data, size_t size) {

	uint8_t result = BSP_W25Qx_Read((uint8_t*)data, (uint32_t)address, (uint32_t)size);

    if (result == 0) {
        return size;  // 读取成功，返回读取的字节数
    } else {
        return -1;     // 读取失败，返回 -1
    }
}

//对w25q64的写函数进行封装，与ringfs文件系统进行链接
ssize_t ringfs_flash_program_wrapper(struct ringfs_flash_partition *flash, int address, const void *data, size_t size) {

	uint8_t result = BSP_W25Qx_Write((uint8_t*)data, (uint32_t)address, (uint32_t)size);

    if (result == 0) {
        return size;  // 写入成功，返回写入的字节数
    } else {
        return -1;     // 写入失败，返回 -1
    }
}

//对w25q64的擦除函数进行封装，与ringfs文件系统进行链接
int ringfs_flash_sector_erase_wrapper(struct ringfs_flash_partition *flash, int address) {

	uint8_t result = BSP_W25Qx_Erase_Block((uint32_t)address);

    // 根据返回值判断擦除是否成功
    if (result == 0) {
        return 0;  // 擦除成功，返回 0
    } else {
        return -1; // 擦除失败，返回 -1
    }
}

struct ringfs_flash_partition flash= {      //这里创建了一个flash实例，并设置了相关的操作函数
    .sector_size = FLASH_SECTOR_SIZE,       //把扇区大小和扇区数量写入flash实例
    .sector_offset = FLASH_PARTITION_OFFSET,
    .sector_count = FLASH_PARTITION_SIZE,

    .sector_erase = ringfs_flash_sector_erase_wrapper,        //设置扇区擦除函数
    .program = ringfs_flash_program_wrapper,      //设置写入函数
    .read = ringfs_flash_read_wrapper,        //设置读取函数
};


