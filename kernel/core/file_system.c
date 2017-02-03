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
	unsigned char zeroBlk[256] = {0,};

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
	int i, j, q, r, blk_off;
	struct inode *pinodeDat;
	char blk_data[256] = {0,};

	/* there is not a directory
	 * file id is same with inode index 
	 * mas file num is limited by inode table size 
	 * whitch is 160
	 */
	if (_file_id >= pfs->inodeTableSize) return 1;

	i = _file_id % pfs->BlkSize;
	q = (int)(i / 8);
	r = i % 8;

	/* if file already exists, return already exist */
	blk_off = (int)(_file_id / pfs->BlkSize);
	read_ramdisk(pfs->inodeBmpStartBlk + blk_off, blk_data);
	if ((blk_data[q] & (0x1 << r)) != 0x00) {
		return 2;
	}
	/* update Inode bitmap */
	blk_data[q] |= (0x01 << r);
	write_ramdisk(pfs->inodeBmpStartBlk + blk_off, blk_data);

	for (i = 0; i < 256; i++) blk_data[i] = 0x00;
	pinodeDat = (struct inode *)(blk_data);
	pinodeDat->iNum = _file_id;
	pinodeDat->fileSize = 0;

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
	int i, j, k, d;
	struct inode iNode;

	i = (int)(_file_id / 8);
	j = _file_id % 8;

	/* file doesn't exist */
	if(!(pfs->pIBmp[i] & (0x1 << j))) return 1;

	/* clear data bitmap */
	read_ramdisk(pfs->inodeTableStartBlk + _file_id, (char *)&iNode);
	k = 0;
	while(iNode.blkloc[k]) {
		d = iNode.blkloc[k] - pfs->DataBlkStartBlk;
		i = (int)(d / 8);
		j = d % 8;
		iNode.blkloc[k] = 0;
		/* clear data bitmap */
		pfs->pDBmp[i] = (pfs->pDBmp[i] & ~(0x1 << j));
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
	int i, j, k, d;
	struct inode iNode;

	i = (int)(_file_id / 8);
	j = _file_id % 8;

	/* file doesn't exist */
	if(!(pfs->pIBmp[i] & (0x1 << j))) return 1;

	/* clear data bitmap */
	read_ramdisk(pfs->inodeTableStartBlk + _file_id, (char *)&iNode);
	k = 0;
	while(iNode.blkloc[k]) {
		d = iNode.blkloc[k] - pfs->DataBlkStartBlk;
		i = (int)(d / 8);
		j = d % 8;
		/* clear data bitmap */
		iNode.blkloc[k] = 0;
		pfs->pDBmp[i] = (pfs->pDBmp[i] & ~(0x1 << j));
		k++;
	}

	iNode.fileSize = 0;
	write_ramdisk(pfs->inodeTableStartBlk + _file_id, (char *)&iNode);
	return 0;
}

void register_file(struct file_system *pfs, struct file *fp)
{
	if(fp->fd >= INODE_TABLE_SIZE) return;
	pfs->fpList[fp->fd] = fp;
}





