/*
 * FileSystem.h
 *
 *  Created on: Oct 23, 2024
 *      Author: 15845
 */

#ifndef INC_FILESYSTEM_H_
#define INC_FILESYSTEM_H_

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

//对文件系统分区的描述，包含着flash的相关信息，以及对flash的操作函数
struct ringfs_flash_partition {
	int sector_size;
	int sector_offset;
	int sector_count;

	//函数指针，指向一个擦除函数，可以将它绑定到任意擦除函数上，可以灵活处理不同闪存的删除操作
	int (*sector_erase)(struct ringfs_flash_partition *flash, int address);

	//函数指针，指向一个写函数，将它绑定到任意编程函数上，这样的话可以灵活处理不同闪存的写入操作
	ssize_t (*program)(struct ringfs_flash_partition *flash, int address, const void *data, size_t size);

	//这是一个函数指针，指向一个函数，这个函数指针的名字是read，我们可以将它绑定到任意读取函数上，这样的话可以灵活处理不同闪存的读取操作
	ssize_t (*read)(struct ringfs_flash_partition *flash, int address, void *data, size_t size);

};

//描述文件系统中位置信息的结构体，可以用来精确描述某个对象在flash中的位置
struct ringfs_loc {
	int sector;     //扇区号
	int slot;       //槽号
};

//文件系统的实例，包含了文件系统的相关信息，以及读写指针等信息
struct ringfs {
	//flash的操作函数的指针（flash的相关信息）
	struct ringfs_flash_partition *flash;

	//每个要存储对象的大小（以字节为单位）
	int object_size;

	//每个扇区的槽数
	int slots_per_sector;

	struct ringfs_loc read;     //读指针，指向要读出的位置
	struct ringfs_loc write;        //写指针，指向要存储的位置
	struct ringfs_loc cursor;       //游标指针，指向当前操作的位置

};

int ringfs_init(struct ringfs *fs, struct ringfs_flash_partition *flash,int object_size);

int ringfs_format(struct ringfs *fs);

int ringfs_scan(struct ringfs *fs);

int ringfs_capacity(struct ringfs *fs);

int ringfs_count_estimate(struct ringfs *fs);

int ringfs_count_exact(struct ringfs *fs);

int ringfs_append(struct ringfs *fs, const void *object);

int ringfs_fetch(struct ringfs *fs, void *object);

int ringfs_discard(struct ringfs *fs);

int ringfs_item_discard(struct ringfs *fs);

int ringfs_rewind(struct ringfs *fs);

#endif /* INC_FILESYSTEM_H_ */
