/*
 * FileSystem_Configuration.h
 *
 *  Created on: Nov 13, 2024
 *      Author: 15845
 */

#ifndef INC_FILESYSTEM_CONFIGURATION_H_
#define INC_FILESYSTEM_CONFIGURATION_H_

#include "FileSystem.h"
#include <stddef.h>
#include "w25q64_ll.h"

#define FLASH_SECTOR_SIZE       4096  //FLASH扇区大小
#define FLASH_TOTAL_SECTORS     2048 //FLASH总扇区数
#define FLASH_PARTITION_OFFSET  101	//FLASH扇区区偏移量
#define FLASH_PARTITION_SIZE    100	//分配给当前文件系统的扇区数


ssize_t ringfs_flash_read_wrapper(struct ringfs_flash_partition *flash, int address, void *data, size_t size);

ssize_t ringfs_flash_program_wrapper(struct ringfs_flash_partition *flash, int address, const void *data, size_t size);

int ringfs_flash_sector_erase_wrapper(struct ringfs_flash_partition *flash, int address);

//flash定义
extern struct ringfs_flash_partition flash;
/* Data record format. */

#endif /* INC_FILESYSTEM_CONFIGURATION_H_ */
