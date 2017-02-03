#ifndef _FILE_SYSTEM_
#define _FILE_SYSTEM_
#include <ramdisk_io.h>
#include <file.h>

/* inode entry number in iNode table */
#define INODETBL_NUM 80
/* physical addr, 48MB from mem start */
#define META_BLK_SIZE		0x1000 /* 4KB block size */
#define SUPER_BLK_START		0x0
#define INODE_BITMAP_START	(SUPER_BLK_START + META_BLK_SIZE)
#define INODE_BITMAP_START_BLK	(INODE_BITMAP_START / DATA_BLK_SIZE)
#define DATA_BLK_BITMAP_START	(INODE_BITMAP_START + META_BLK_SIZE)
#define DATA_BLK_BITMAP_START_BLK	(DATA_BLK_BITMAP_START / DATA_BLK_SIZE)
#define INODE_TABLE_START	(DATA_BLK_BITMAP_START +META_BLK_SIZE)
#define INODE_TABLE_START_BLK	(INODE_TABLE_START / DATA_BLK_SIZE)
#define INODE_TABLE_SIZE	(INODETBL_NUM * DATA_BLK_SIZE) /* 20KB = 5 blocks = 80 entries */
#define DATA_BLK_START		(INODE_TABLE_START + INODE_TABLE_SIZE)
#define DATA_BLK_START_BLK	(DATA_BLK_START / DATA_BLK_SIZE)

/* inode size is same as 1blk size(256 byte) */
struct inode {
	unsigned int iNum;
	unsigned int fileSize;
	/* max file size has 50 data blks
	   file size = 256 * 50 bytes
	 */
	/* index of data blk */
	unsigned int blkloc[50]; 
};

struct file_system {
	unsigned char *pIBmp;
	unsigned char *pDBmp;
	unsigned char *SuperBlkStart;
	unsigned int SuperBlkStartBlk;
	unsigned char *inodeBmpStart;
	unsigned int inodeBmpStartBlk;
	unsigned char *DataBmpStart;
	unsigned int DataBmpStartBlk;
	unsigned char *inodeTableStart;
	unsigned int inodeTableStartBlk;
	unsigned int inodeTableSize; /* 256 byte blk size */
	unsigned char *DataBlkStart;
	unsigned int DataBlkStartBlk;
	int bMounted;
	/*MirrorDisk *pBlkingDisk;*/
	/* kludge but simple */
	struct file *fpList[64];
	unsigned int BlkSize;
};

struct file_system *init_file_system();
int format_file_system(struct file_system *pfs);
void mount_file_system(struct file_system *pfs);
int release_blks(struct file_system *pfs, int _file_id);
int find_datablk(struct file_system *pfs);
void register_file(struct file_system *pfs, struct file *fp);
int file_system_create_file(struct file_system *pfs, int _file_id);
#endif
