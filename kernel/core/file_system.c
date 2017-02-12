#include <ramdisk_io.h>
#include <file_system.h>
#include <smm.h>

struct file_system *init_file_system()
{
	int i;
	struct file_system *pfs;
	pfs = (struct file_system *)kmalloc(sizeof(struct file_system));

	/* TO DO : currently no information in superblock */
	pfs->bMounted = 0;
	/* max file num */
	pfs->inodeTableSize = 64;
	pfs->BlkSize = DATA_BLK_SIZE;
	pfs->pIBmp = pfs->pDBmp = 0;

	for (i = 0; i < INODE_TABLE_SIZE; i++) {
		pfs->fpList[i] = 0;
	}
	return pfs;
}

void mount_file_system(struct file_system *pfs)
{
	pfs->SuperBlkStartBlk = SUPER_BLK_START;
	pfs->inodeBmpStartBlk = INODE_BITMAP_START_BLK;
	pfs->DataBmpStartBlk = DATA_BLK_BITMAP_START_BLK;
	pfs->inodeTableStartBlk = INODE_TABLE_START_BLK;
	pfs->DataBlkStartBlk = DATA_BLK_START_BLK;

	pfs->pIBmp = (unsigned char *)(RAMDISK_START + INODE_BITMAP_START);
	pfs->pDBmp = (unsigned char *)(RAMDISK_START + DATA_BLK_BITMAP_START);

	pfs->bMounted = 1;
}

int format_file_system(struct file_system *pfs)
{
	int i;
	unsigned char zeroBlk[DATA_BLK_SIZE] = {0,};

	/* wipe out all the data in the disk, 
	 * but for simplicity, wipe out just meta blks
	 * which are the first 104 blks
	 */
	for (i = pfs->inodeBmpStartBlk; 
		i < TOTAL_BLK_NUM - pfs->inodeBmpStartBlk; i++) {
		write_ramdisk(i, zeroBlk);
	}

	return 0;
}

int file_system_create_file(struct file_system *pfs, int _file_id)
{
	int i, j, blk_off, blk_r, bit;
	struct inode *pinodeDat;
	char blk_data[DATA_BLK_SIZE] = {0,};

	/* there is not a directory
	 * file id is same with inode index 
	 * mas file num is limited by inode table size 
	 * whitch is 160
	 */
	if (_file_id >= pfs->inodeTableSize * 8) return 1;

	blk_off = (int)((_file_id >> 3) / pfs->BlkSize);
	blk_r = (_file_id >> 3) % pfs->BlkSize;
	bit = _file_id % 8;

	/* if file already exists, return already exist */
	read_ramdisk(pfs->inodeBmpStartBlk + blk_off, blk_data);
	if ((blk_data[blk_r] & (0x1 << bit)) != 0x00) {
		return 2;
	}
	/* update Inode bitmap */
	blk_data[blk_r] |= (0x01 << bit);
	write_ramdisk(pfs->inodeBmpStartBlk + blk_off, blk_data);

	for (i = 0; i < DATA_BLK_SIZE; i++) blk_data[i] = 0x00;
	pinodeDat = (struct inode *)(blk_data);
	pinodeDat->iNum = _file_id;
	pinodeDat->file_size = 0;

	/* write inode data for new file to inode table */
	write_ramdisk(pfs->inodeTableStartBlk + _file_id, blk_data);

	return 0;
}

int find_datablk(struct file_system *pfs)
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

struct file *find_file(struct file_system *pfs, int _file_id)
{
	unsigned char ib;
	int i, j;

	i = (int)(_file_id / 8);
	j = _file_id % 8;

	if (pfs->pIBmp[i] & (0x1 << j)) {
		return pfs->fpList[_file_id];
	} else return 0;
}

int delete_file(struct file_system *pfs, int _file_id)
{
	int i, j, k, l, d, i2, j2, d2;
	char temp[DATA_BLK_SIZE];
	struct inode iNode;

	i = (int)(_file_id / 8);
	j = _file_id % 8;

	/* file doesn't exist */
	if(!(pfs->pIBmp[i] & (0x1 << j))) return 1;

	/* clear data bitmap */
	read_ramdisk(pfs->inodeTableStartBlk + _file_id, (char *)&iNode);
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

	i = (int)(_file_id / 8);
	j = _file_id % 8;
	/* clear inode bitmap */
	pfs->pIBmp[i] = (pfs->pIBmp[i] & ~(0x1 << j));

	return 0;
}

int release_blks(struct file_system *pfs, int _file_id)
{
	int i, j, k, l, d, i2, j2, d2;
	char temp[DATA_BLK_SIZE];
	struct inode iNode;

	i = (int)(_file_id / 8);
	j = _file_id % 8;

	/* file doesn't exist */
	if(!(pfs->pIBmp[i] & (0x1 << j))) return 1;

	/* clear data bitmap */
	read_ramdisk(pfs->inodeTableStartBlk + _file_id, (char *)&iNode);
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
	write_ramdisk(pfs->inodeTableStartBlk + _file_id, (char *)&iNode);
	return 0;
}

void register_file(struct file_system *pfs, struct file *fp)
{
	if(fp->fd >= INODE_TABLE_SIZE) return;
	pfs->fpList[fp->fd] = fp;
}

