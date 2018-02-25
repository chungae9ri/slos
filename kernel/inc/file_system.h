/*
  kernel/core/gic.c general interrupt controller 
  (C) 2018 Kwangdo Yi <kwangdo.yi@gmail.com>
 
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, see <http://www.gnu.org/licenses/>
*/

#ifndef _FILE_SYSTEM_
#define _FILE_SYSTEM_
#include <file.h>
#include <stdint-gcc.h>

/* inode entry number in iNode table */
#define INODE_NUM 			8 /* 8 = 4KB / 512B*/	
/* physical addr, 48MB from mem start */
#define META_BLK_SIZE			0x1000 /* 4KB meta block size */
#define DATA_BLK_SIZE			0x200  /* data block size 512B */
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

#define INODEBLKMAX			((DATA_BLK_SIZE - sizeof(uint32_t) * 2)	 / sizeof(uint32_t))
/* inode size is same as 1blk size(512 byte) */
struct inode {
	uint32_t iNum;
	uint32_t file_size;

	/* index of data blk max file size = 63KB  = 126(entries) *  512 B */
	uint32_t blkloc[INODEBLKMAX]; 
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
int register_file(struct file *fp);
int unregister_file(struct file *fp);
int32_t file_system_create_file(struct file *fp);
uint32_t delete_file(struct file *fp);
#endif
