/*
 * FileSystem.c
 *
 *  Created on: Oct 23, 2024
 *      Author: 15845
 */


#include "FileSystem.h"
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include "w25q64_ll.h"


enum sector_status {
    SECTOR_ERASED     = 0xFFFFFFFF, //擦除过后，扇区的默认状态
    SECTOR_FREE       = 0xFFFFFF00, //扇区已经擦除，且目前是空闲的
    SECTOR_IN_USE     = 0xFFFF0000, //扇区当前包含有效数据
    SECTOR_ERASING    = 0xFF000000, //扇区正处于将要被擦除的过程中
    SECTOR_FORMATTING = 0x00000000, //整个分区正在格式化
};

struct sector_header {      //扇区头部结构体，用于记录扇区的状态和版本号
    uint32_t status;    //扇区状态，可以从sector_status查询当前扇区的状态
};

//获取当前扇区的绝对地址
static int _sector_address(struct ringfs *fs, int sector_offset){
	/*fs->flash->sector_offset表示文件系统分区占用的扇区，sector_offset表示当前扫描到的扇区号
	    上面两个数相加能得到当前要扫描的扇区的绝对地址，乘每个扇区的大小就能得到当前扇区的物理地址
	    */
	return (fs->flash->sector_offset + sector_offset) * fs->flash->sector_size;
}

//获取当前扇区的状态
static int _sector_get_status(struct ringfs *fs, int sector, uint32_t *status)
{

    return fs->flash->read(fs->flash,_sector_address(fs, sector) + offsetof(struct sector_header, status),status, sizeof(*status));
}

//设置扇区状态
static int _sector_set_status(struct ringfs *fs, int sector, uint32_t status){

	return fs->flash->program(fs->flash,_sector_address(fs, sector) + offsetof(struct sector_header, status),&status, sizeof(status));
}

//擦除一个扇区并将其标记为空闲状态
static int _sector_free(struct ringfs *fs, int sector)
{
    int sector_addr = _sector_address(fs, sector);//获取当前扇区的绝对地址
    _sector_set_status(fs, sector, SECTOR_ERASING);     //设置当前扇区的状态为正在擦除
    fs->flash->sector_erase(fs->flash, sector_addr);        //擦除当前扇区
//    fs->flash->program(fs->flash,
//            sector_addr + offsetof(struct sector_header, version),
//            &fs->version, sizeof(fs->version));     //写入版本号
    _sector_set_status(fs, sector, SECTOR_FREE);        //设置当前扇区的状态为空闲
    return 0;
}

//设置槽的状态，防止意外掉电
enum slot_status {
    SLOT_ERASED   = 0xFFFFFFFF, //擦除后空闲，可以写入
    SLOT_RESERVED = 0xFFFFFF00, //正在写入，但未提交，暂时不可用
    SLOT_VALID    = 0xFFFF0000, //写入完成，包含有效数据
    SLOT_GARBAGE  = 0xFF000000, //数据已无效，槽可回收或重用
};

struct slot_header {        //槽头部结构体，用于记录槽的状态
    uint32_t status;
};

static int _slot_address(struct ringfs *fs, struct ringfs_loc *loc) {
	return _sector_address(fs, loc->sector) +sizeof(struct sector_header) + (sizeof(struct slot_header) + fs->object_size) * loc->slot;
}

//获取槽的状态
static int _slot_get_status(struct ringfs *fs, struct ringfs_loc *loc, uint32_t *status)
{
    /*
    将当前槽的状态读入status变量中
    */
    return fs->flash->read(fs->flash,_slot_address(fs, loc) + offsetof(struct slot_header, status),status, sizeof(*status));
}

static int _slot_set_status(struct ringfs *fs, struct ringfs_loc *loc, uint32_t status)
{
    return fs->flash->program(fs->flash, _slot_address(fs, loc) + offsetof(struct slot_header, status),&status, sizeof(status));
}

static bool _loc_equal(struct ringfs_loc *a, struct ringfs_loc *b)
{
    return (a->sector == b->sector) && (a->slot == b->slot);
}

static void _loc_advance_sector(struct ringfs *fs, struct ringfs_loc *loc){
	loc->slot = 0;
	loc->sector++;
	if (loc->sector >= fs->flash->sector_count){
		loc->sector = 0;
	}
}

static void _loc_advance_slot(struct ringfs *fs, struct ringfs_loc *loc)
{
    loc->slot++;
    if (loc->slot >= fs->slots_per_sector){//如果到达了当前扇区的最后一个槽，那么就需要移动到下一个扇区
        _loc_advance_sector(fs, loc);
    }
}

//ringfs文件系统初始化函数
int ringfs_init(struct ringfs *fs, struct ringfs_flash_partition *flash, int object_size){
	fs->flash = flash;
	fs->object_size = object_size;

	fs->slots_per_sector = (fs->flash->sector_size - sizeof(struct sector_header)) / (sizeof(struct slot_header) + fs->object_size);

	return 0;
}

//ringfs文件系统格式化函数
int ringfs_format(struct ringfs *fs){
	//首先让全部的扇区都设置为正在格式化状态，这样做是为了确保不会出现半格式化的情况
	for (int sector=0; sector<fs->flash->sector_count; sector++){
		_sector_set_status(fs, sector, SECTOR_FORMATTING);
	}

	//再让所有扇区设置为空闲状态
	for (int sector=0; sector<fs->flash->sector_count; sector++){
		_sector_free(fs, sector);
	}

	//将所有指针的所有内容均格式化为0
	fs->read.sector = 0;
	fs->read.slot = 0;
	fs->write.sector = 0;
	fs->write.slot = 0;
	fs->cursor.sector = 0;
	fs->cursor.slot = 0;

	return 0;
}

//启动时扫描整个文件系统
int ringfs_scan(struct ringfs *fs){
	uint32_t previous_sector_status = SECTOR_FREE;      //上一个扇区的状态，设置为空闲状态

	int read_sector = 0;        //第一个扇区，或者第一个在使用的扇区即读扇区

	int write_sector = fs->flash->sector_count - 1;     //最后一个扇区，或者最后一个在使用的扇区即写扇区

	bool free_seen = false;     //用于记录是否见过空闲扇区，初始化为 false

	bool used_seen = false;     //用于记录是否见过在使用的扇区，初始化为 false

	//遍历整个扇区
	for (int sector=0; sector<fs->flash->sector_count; sector++){
		int addr = _sector_address(fs, sector);     //得到当前扇区的起始物理地址

		struct sector_header header;        //扇区头部对象

		//读取当前扇区的头部，并赋值到header变量中
		fs->flash->read(fs->flash, addr, &header, sizeof(header));

		if (header.status == SECTOR_FORMATTING) {       //如果检测到当前扇区正在格式化，则返回错误
			return -1;
		}

		if (header.status == SECTOR_ERASING || header.status == SECTOR_ERASED) {
		            _sector_free(fs, addr); //擦除当前扇区并将其标记为空闲状态
		            header.status = SECTOR_FREE;
		}

		//检查当前扇区头部，如果状态既不是空闲也不是在使用中，那么这个扇区就错误
		if (header.status != SECTOR_FREE && header.status != SECTOR_IN_USE) {
			return -1;
		}

        //检测版本号是否与文件系统中的版本号一致
//        if (header.version != fs->version) {
//            return -1;
//        }

		//如果遇到了一个空闲块，就把free_seen设置为true，表示已经至少有一个空闲扇区
		if (header.status == SECTOR_FREE){
			free_seen = true;
		}

		//如果遇到了一个正在使用的块，就把used_seen设置为true，表示已经至少有一个正在使用的块
		if (header.status == SECTOR_IN_USE){
			used_seen = true;
		}

		//更新读扇区的值，如果当前扇区在使用状态，前一个扇区为空闲状态，那么就让read_sector为当前扇区号
		if (header.status == SECTOR_IN_USE && previous_sector_status == SECTOR_FREE){
			read_sector = sector;
		}

		//更新写扇区的值，如果当前扇区为空闲状态，前一个扇区在使用状态，那么就让write_sector为当前扇区号-1
		if (header.status == SECTOR_FREE && previous_sector_status == SECTOR_IN_USE) {
			write_sector = sector-1;
		}

		previous_sector_status = header.status;     //更新前一个扇区的状态为当前扇区的状态，开始下一轮循环

	}

	//如果没有见到空闲扇区，则返回错误
	if (!free_seen) {
		return -1;
	}

	//如果没有在使用的扇区，则把写扇区设置为第一个扇区
	if (!used_seen) {
		write_sector = 0;
	}

	fs->write.sector = write_sector;        //把扫描过后的第一个可以写入的扇区号赋值给写扇区成员变量
	fs->write.slot = 0;     //把写槽设置为0，表示可以从头开始写入

	//遍历当前扇区的所有槽，当扇区号变化则代表当前扇区所有槽都满了
	while (fs->write.sector == write_sector) {
		uint32_t status;        //用来记录当前槽的状态
		_slot_get_status(fs, &fs->write, &status);//读取当前槽的状态并存入status变量
		if (status == SLOT_ERASED) {//如果当前槽状态为已经擦除，那么就找到了第一个可以写入的槽，就可以跳出循环
			 break;
		}
		_loc_advance_slot(fs, &fs->write);  //跳到下一个槽
	}

	fs->read.sector = read_sector;      //把之前扫描得到的第一个正在使用的扇区号赋值给读扇区成员变量
	fs->read.slot = 0;      //把读槽设置为0，表示从头开始读取

	while (!_loc_equal(&fs->read, &fs->write)) {
		uint32_t status;        //用来存储当前槽的状态
		_slot_get_status(fs, &fs->read, &status);       //读取当前槽的状态并存入status变量
		if (status == SLOT_VALID){     //如果当前槽状态为有效，则表示找到了有效数据，就可以跳出循环
			break;
		}
		_loc_advance_slot(fs, &fs->read);       //跳到下一个槽
	}

	fs->cursor = fs->read;      //将读指针指向赋值给游标指针

	return 0;
}

int ringfs_capacity(struct ringfs *fs)
{
    return fs->slots_per_sector * (fs->flash->sector_count - 1);
}

//估算当前文件系统已经存储的数据量
int ringfs_count_estimate(struct ringfs *fs)
{

    int sector_diff = (fs->write.sector - fs->read.sector + fs->flash->sector_count) %
        fs->flash->sector_count;

    return sector_diff * fs->slots_per_sector + fs->write.slot - fs->read.slot;
}

//计算当前文件系统中有效数据项的精确数量
int ringfs_count_exact(struct ringfs *fs)
{
    int count = 0;

    /* Use a temporary loc for iteration. */
    struct ringfs_loc loc = fs->read;
    while (!_loc_equal(&loc, &fs->write)) {
        uint32_t status;
        _slot_get_status(fs, &loc, &status);

        if (status == SLOT_VALID)
            count++;

        _loc_advance_slot(fs, &loc);
    }

    return count;
}

int ringfs_append(struct ringfs *fs, const void *object){
	uint32_t status;

	//计算出当前写入扇区的下一个扇区
	int next_sector = (fs->write.sector+1) % fs->flash->sector_count;

	//得到下一个扇区的状态
	_sector_get_status(fs, next_sector, &status);

	if (status != SECTOR_FREE) {
		if (fs->read.sector == next_sector){
			_loc_advance_sector(fs, &fs->read);
		}
		if (fs->cursor.sector == next_sector){
				_loc_advance_sector(fs, &fs->cursor);
			}

		_sector_free(fs, next_sector);
	}
	//提升存储性能测试代码
//	if(fs->write.slot == 0){
//		while(BSP_W25Qx_GetStatus() == W25Qx_BUSY);
//	}

	//得到当前要写入扇区的状态
	_sector_get_status(fs, fs->write.sector, &status);

	if (status == SECTOR_FREE) {
		//把当前扇区设为待使用状态
		_sector_set_status(fs, fs->write.sector, SECTOR_IN_USE);
	}
	else if(status != SECTOR_IN_USE){
		return -1;
	}

	 _slot_set_status(fs, &fs->write, SLOT_RESERVED);

	 //将数据对象写入到
	 fs->flash->program(fs->flash,_slot_address(fs, &fs->write) + sizeof(struct slot_header),object, fs->object_size);

	 _slot_set_status(fs, &fs->write, SLOT_VALID);

	 _loc_advance_slot(fs, &fs->write);

	 return 0;
}

//使用游标指针，从read指针到write指针按顺序读取每一个对象
int ringfs_fetch(struct ringfs *fs, void *object)
{

    while (!_loc_equal(&fs->cursor, &fs->write)) {
        uint32_t status;

        _slot_get_status(fs, &fs->cursor, &status);

        if (status == SLOT_VALID) {
            fs->flash->read(fs->flash,
                    _slot_address(fs, &fs->cursor) + sizeof(struct slot_header),
                    object, fs->object_size);
            _loc_advance_slot(fs, &fs->cursor);
            return 0;
        }

        _loc_advance_slot(fs, &fs->cursor);
    }

    return -1;
}

//垃圾回收机制，把read指针和游标指针之间的所有槽设置为垃圾状态
int ringfs_discard(struct ringfs *fs)
{
    while (!_loc_equal(&fs->read, &fs->cursor)) {
        _slot_set_status(fs, &fs->read, SLOT_GARBAGE);
        _loc_advance_slot(fs, &fs->read);
    }

    return 0;
}

//垃圾回收机制，把单个槽设置为垃圾状态
int ringfs_item_discard(struct ringfs *fs)
{
        _slot_set_status(fs, &fs->read, SLOT_GARBAGE);
        _loc_advance_slot(fs, &fs->read);

    return 0;
}

//将游标指针 (fs->cursor) 重置为读取指针 (fs->read) 的位置,实现文件系统中的回溯或者重新读取
int ringfs_rewind(struct ringfs *fs)
{
    fs->cursor = fs->read;
    return 0;
}




