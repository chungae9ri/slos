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

#include <stdio.h>
#include <ramdisk_io.h>
#include <file_system.h>
#include <mm.h>

struct file_system *pfs;

struct file_system *init_file_system()
{
	int i;
	pfs = (struct file_system *)kmalloc(sizeof(struct file_system));

	/* TO DO : currently no information in superblock */
	pfs->bMounted = 0;
	/* max file num */
	pfs->inodeTableSize = INODE_NUM;
	pfs->BlkSize = DATA_BLK_SIZE;
	pfs->pIBmp = pfs->pDBmp = 0;
	pfs->fdCnt = 0;

	for (i = 0; i < INODE_TABLE_SIZE; i++) {
		pfs->fpList[i] = NULL;
	}
	return pfs;
}

void mount_file_system(void)
{
	pfs->SuperBlkStartBlk = SUPER_BLK_START_BLK;
	pfs->inodeBmpStartBlk = INODE_BITMAP_START_BLK;
	pfs->DataBmpStartBlk = DATA_BLK_BITMAP_START_BLK;
	pfs->inodeTableStartBlk = INODE_TABLE_START_BLK;
	pfs->DataBlkStartBlk = DATA_BLK_START_BLK;

	pfs->pIBmp = (unsigned char *)(RAMDISK_START + INODE_BITMAP_START);
	pfs->pDBmp = (unsigned char *)(RAMDISK_START + DATA_BLK_BITMAP_START);

	pfs->bMounted = 1;
}

int32_t format_file_system(void)
{
	int i;
	unsigned char zeroBlk[DATA_BLK_SIZE] = {0,};

	/*
	 * wipe out all the data in the disk, 
	 * but for simplicity, wipe out just meta blks
	 * which are the first 104 blks
	 */
	for (i = pfs->inodeBmpStartBlk; 
		i < TOTAL_BLK_NUM - pfs->inodeBmpStartBlk; i++) {
		write_ramdisk(i, (char *)zeroBlk);
	}

	return 0;
}

int32_t file_system_create_file(struct file *fp)
{
	int i, blk_off, byte, bit, inodeIdx;
	struct inode *pinodeDat;
	char blk_data[DATA_BLK_SIZE] = {0,};

	inodeIdx = fp->fd;

	/*
	 * there is not a directory
	 * file id is same with inode index 
	 * max file num is limited by inode table size 
	 * which is 80
	 */
	if (inodeIdx >= INODE_NUM) {
		return 1;
	}

	/*
	 * Since every disk RW is done by block, 
	 * need to calculate the block offset(quotient).
	 * But our max file_id is 80, the quotient is 
	 * always 0. 
	 */
	blk_off = (int)((inodeIdx >> 3) / pfs->BlkSize);
	byte = (inodeIdx>> 3) % pfs->BlkSize;
	bit = inodeIdx % 8;

	/* if file already exists, return already exist */
	read_ramdisk(pfs->inodeBmpStartBlk + blk_off, blk_data);
	if ((blk_data[byte] & (0x1 << bit)) != 0x00) {
		return 2;
	}
	/* update Inode bitmap */
	blk_data[byte] |= (0x01 << bit);
	write_ramdisk(pfs->inodeBmpStartBlk + blk_off, blk_data);

	for (i = 0; i < DATA_BLK_SIZE; i++) {
		blk_data[i] = 0x00;
	}
	pinodeDat = (struct inode *)(blk_data);
	pinodeDat->iNum = inodeIdx;
	pinodeDat->file_size = 0;

	/* write inode data for new file to inode table */
	write_ramdisk(pfs->inodeTableStartBlk + inodeIdx, blk_data);

	return 0;
}

int32_t find_datablk(void)
{
	int i, j, DataBlkIdx = -1;

	/* search dbmp to find out empty data block
	 * to store the file data 
	 */
	for (i = 0; i < META_BLK_SIZE; i++) {
		for (j = 0; j < 8; j++) {
			if ((pfs->pDBmp[i] & (0x1 << j)) == 0x00) {
				DataBlkIdx = 8 * i + j;
				/* update pDbmp */
				pfs->pDBmp[i] |= (0x1 << j);
				break;
			}
		}
		if(DataBlkIdx != -1) break;
	}

	if (i == META_BLK_SIZE && j == 8) {
		return -1;
	} else {
		return (int)(pfs->DataBlkStartBlk + DataBlkIdx);
	}
}

struct file *find_file(struct file *fp)
{
	uint32_t _file_id;

	_file_id = fp->fd;

	if (pfs->fpList[_file_id]) {
		return pfs->fpList[_file_id];
	} else return NULL;
}

uint32_t delete_file(struct file *fp)
{
	int i, j, k, l, d, i2, j2, d2, inodeIdx;
	char temp[DATA_BLK_SIZE];
	struct inode iNode;

	inodeIdx = fp->fd;

	i = (int)(inodeIdx / 8);
	j = inodeIdx % 8;

	/* file doesn't exist */
	if(!(pfs->pIBmp[i] & (0x1 << j))) return 1;

	/* clear data bitmap */
	read_ramdisk(pfs->inodeTableStartBlk + inodeIdx, (char *)&iNode);
	k = 0;
	while(iNode.blkloc[k]) {
		d = iNode.blkloc[k];
		i = (int)(d / 8);
		j = d % 8;
		/* read 2nd data block and clear it*/
		read_ramdisk(d, temp);
		for (l = 0; l < DATA_BLK_SIZE / sizeof(int); l++) {
			d2 = temp[l * 4];
			i2 = (int)(d2 / 8);
			j2 = d2 % 8;
			pfs->pDBmp[i2] = (pfs->pDBmp[i2] & ~(0x1 << j2));
		}
		/* clear 1st data bitmap */
		pfs->pDBmp[i] = (pfs->pDBmp[i] & ~(0x1 << j));
		/* clear inode blkloc */
		iNode.blkloc[k] = 0;
		k++;
	}

	i = (int)(inodeIdx / 8);
	j = inodeIdx % 8;
	/* clear inode bitmap */
	pfs->pIBmp[i] = (pfs->pIBmp[i] & ~(0x1 << j));

	return 0;
}

int32_t release_blks(struct file *fp)
{
	int i, j, k, l, d, i2, j2, d2, inodeIdx;
	char temp[DATA_BLK_SIZE];
	struct inode iNode;

	inodeIdx = fp->fd;
	i = (int)(inodeIdx/ 8);
	j = inodeIdx % 8;

	/* file doesn't exist */
	if(!(pfs->pIBmp[i] & (0x1 << j))) return 1;

	/* clear data bitmap */
	read_ramdisk(pfs->inodeTableStartBlk + inodeIdx, (char *)&iNode);
	k = 0;
	while(iNode.blkloc[k]) {
		d = iNode.blkloc[k];
		i = (int)(d / 8);
		j = d % 8;
		/* read 2nd data block and clear it*/
		read_ramdisk(d, temp);
		for (l = 0; l < DATA_BLK_SIZE / sizeof(int); l++) {
			d2 = temp[l * 4];
			i2 = (int)(d2 / 8);
			j2 = d2 % 8;
			pfs->pDBmp[i2] = (pfs->pDBmp[i2] & ~(0x1 << j2));
		}
		/* clear 1st data bitmap */
		pfs->pDBmp[i] = (pfs->pDBmp[i] & ~(0x1 << j));
		/* clear inode blkloc */
		iNode.blkloc[k] = 0;
		k++;
	}

	iNode.file_size = 0;
	write_ramdisk(pfs->inodeTableStartBlk + inodeIdx, (char *)&iNode);
	return 0;
}

int32_t register_file(struct file *fp)
{
	int i;

	if(pfs->fdCnt >= INODE_TABLE_SIZE) {
		return -1;
	}

	for (i = 0; i < INODE_NUM; i++) {
		if (pfs->fpList[i] == NULL) 
			break;
	}

	pfs->fpList[i] = fp; 
	/* the location in the fpList is the fd value(0 ~ 79) */
	return i;
}

int32_t unregister_file(struct file *fp)
{
	uint32_t _file_id;

	_file_id = fp->fd;
	if(_file_id >= INODE_TABLE_SIZE) {
		return 1;
	}

	pfs->fpList[_file_id] = NULL;

	return 0;
}
