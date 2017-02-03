#include <ramdisk_io.h>
#include <task.h>

void read_ramdisk(int mem_blk_num, char *buf)
{
	int i;

	for (i = 0; i < DATA_BLK_SIZE; i++) {
		buf[i] = ((char *)(RAMDISK_START + mem_blk_num * DATA_BLK_SIZE))[i];
	}
}

void write_ramdisk(int mem_blk_num, char *buf)
{
	int i;

	for (i = 0; i < DATA_BLK_SIZE; i++) {
		((char *)(RAMDISK_START + mem_blk_num * DATA_BLK_SIZE))[i] = buf[i];
	}
}

