//FS_ExampleHandle = osThreadNew(FS_Example, NULL, &FS_Example_attributes);
#include "Flash_FS_Example.h"
void FS_Example(void *argument){
	struct log_entry {      //定义日志条目结构体
			uint32_t timestamp;  // 时间戳
			char data5[90];     // 90 bytes
			float data4[20];    // 80 bytes
			uint32_t data3[10]; // 40 bytes
	//		uint16_t data2[20]; // 40 bytes
		};

	struct ringfs fs;       //定义ringfs结构体变量fs，这就是一个文件系统的实例
	ringfs_init(&fs, &flash, sizeof(struct log_entry));      //初始化文件系统

	//扫描
	int a = 0;
	int c = 0;
	if (ringfs_scan(&fs) != 0) {
		ringfs_format(&fs);     //格式化文件系统
	}

	struct log_entry entry;
	a = ringfs_count_estimate(&fs);
	c = ringfs_capacity(&fs);
	uint32_t start_time,write_time;
	start_time = xTaskGetTickCount();
	for (int i = 0; i < 1000; i++) {
		memset(entry.data5, 'A', sizeof(entry.data5)); // 将 data5 数组的每个元素都设置为 'A'
		for (int i = 0; i < 20; i++) {
		    entry.data4[i] = 3.14f;
		}
		for (int i = 0; i < 10; i++) {
		    entry.data3[i] = 12345678;
		}
		write_time = xTaskGetTickCount();
		entry.timestamp = write_time;
		ringfs_append(&fs, &entry);

	}
	a = ringfs_count_estimate(&fs);
	c = ringfs_capacity(&fs);
	struct log_entry entry1;
	for(int i = 0;i<100;i++){
		assert(ringfs_fetch(&fs, &entry1) == 0);
	}



	while(true){
	    	vTaskDelay(100);
	}
}
