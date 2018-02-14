#ifndef _FILE_SYSTEM_
#define _FILE_SYSTEM_
#include <file.h>
#include <stdint-gcc.h>

/* inode entry number in iNode table */
#define INODE_NUM 			80
/* physical addr, 48MB from mem start */
#define META_BLK_SIZE			0x1000 /* 4KB meta block size */
#define DATA_BLK_SIZE			0x100
#define SUPER_BLK_START			0x0
#define SUPER_BLK_START_BLK		(SUPER_BLK_START / DATA_BLK_SIZE)
#define INODE_BITMAP_START		(SUPER_BLK_START + META_BLK_SIZE)
#define INODE_BITMAP_START_BLK		(INODE_BITMAP_START / DATA_BLK_SIZE)
#define DATA_BLK_BITMAP_START		(INODE_BITMAP_START + META_BLK_SIZE)
#define DATA_BLK_BITMAP_START_BLK	(DATA_BLK_BITMAP_START / DATA_BLK_SIZE)
#define INODE_TABLE_START		(DATA_BLK_BITMAP_START + META_BLK_SIZE)
#define INODE_TABLE_START_BLK		(INODE_TABLE_START / DATA_BLK_SIZE)
#define INODE_TABLE_SIZE		(INODE_NUM * DATA_BLK_SIZE)
#define DATA_BLK_START			(INODE_TABLE_START + INODE_TABLE_SIZE)
#define DATA_BLK_START_BLK		(DATA_BLK_START / DATA_BLK_SIZE)

#define inodeBlkMax			50
/* inode size is same as 1blk size(256 byte) */
struct inode {
	uint32_t iNum;
	uint32_t file_size;

	/* index of data blk max file size = 800KB = 50 * 256 / 4 * 256 B */
	uint32_t blkloc[inodeBlkMax]; 
};

struct file_system {
	uint8_t *pIBmp;
	uint8_t *pDBmp;
	uint8_t *SuperBlkStart;
	uint32_t SuperBlkStartBlk;
	uint8_t *inodeBmpStart;
	uint32_t inodeBmpStartBlk;
	uint8_t *DataBmpStart;
	uint32_t DataBmpStartBlk;
	uint8_t *inodeTableStart;
	uint32_t inodeTableStartBlk;
	uint32_t inodeTableSize; /* 256 byte blk size */
	uint8_t *DataBlkStart;
	uint32_t DataBlkStartBlk;
	uint32_t bMounted;
	uint32_t fdCnt;
	struct file *fpList[INODE_NUM];
	uint32_t BlkSize;
};

struct file_system *init_file_system();
int32_t format_file_system(void);
void mount_file_system(void);
int32_t release_blks(struct file *fp);
int32_t find_datablk(void);
int32_t register_file(struct file *fp);
int32_t unregister_file(struct file *fp);
int32_t file_system_create_file(struct file *fp);
uint32_t delete_file(struct file *fp);
#endif
