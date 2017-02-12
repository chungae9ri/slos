#ifndef _RAMDISK_IO_
#define _RAMDISK_IO_

#define DATA_BLK_SIZE		0x100
#define RAMDISK_START		0x3000000  /* 48MB */
#define RAMDISK_SIZE		0x800000 /* 8MB */
#define TOTAL_BLK_NUM		(RAMDISK_SIZE / DATA_BLK_SIZE)

void write_ramdisk(int mem_blk_num, char *buf);
void read_ramdisk(int mem_blk_num, char *buf);
#endif
