#ifndef _RAMDISK_IO_
#define _RAMDISK_IO_

#include <file_system.h>

#define RAMDISK_START		0x03000000  /* 48MB */
#define RAMDISK_SIZE		0x400000 /* 4MB */
#define TOTAL_BLK_NUM		(RAMDISK_SIZE / DATA_BLK_SIZE)

void write_ramdisk(int mem_blk_num, char *buf);
void read_ramdisk(int mem_blk_num, char *buf);
#endif
