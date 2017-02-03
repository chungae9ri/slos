#include <file.h>
#include <file_system.h>
#include <smm.h>
#include <string.h>

extern struct file_system *pfs;

struct file *create_file(int _fd, char *str)
{
	int i;
	struct file *fp = (struct file *)kmalloc(sizeof(struct file));

	fp->fd = _fd;
	file_system_create_file(pfs, _fd);
	fp->pos = 0;
	strcpy(fp->name, str);
	register_file(pfs, fp);

	return fp;
}

int read(struct file *fp, int _n, char *_buf)
{
	char temp[256];
	int i, j;
	int inodeLoc, BlkIdx, BlkNum, off, r;
	struct inode iNode;
	int BlkSize = DATA_BLK_SIZE;

	BlkIdx = (int)(fp->pos / BlkSize);
	off = (int)(fp->pos % BlkSize); 

	/* read inode
	 * since there is not a root directory, fd is the index
	 * of the file entry to inode table
	 */
	inodeLoc = pfs->inodeTableStartBlk + fp->fd;
	read_ramdisk(inodeLoc, (char *)(&iNode));

	BlkNum = iNode.blkloc[BlkIdx];
	read_ramdisk(BlkNum, temp);

	if (off + _n < BlkSize) {
		for(i = 0; i < _n; i++) {
			_buf[i] = temp[off + i];
		}
	} else {
		for (i = 0; i < BlkSize-off; i++) {
			_buf[i] = temp[off + i];
		}

		r = _n - i;
		BlkNum = (unsigned long)(r / BlkSize);
		off = (unsigned int)(r % BlkSize);
		BlkIdx++;
		for (j = 0; j < BlkNum; j++) {
			read_ramdisk(iNode.blkloc[BlkIdx], &_buf[i]);
			i += BlkSize;
			BlkIdx++;
		}

		if (off) {
			read_ramdisk(iNode.blkloc[BlkIdx], temp);
			for (j = 0; j < off; j++) {
				_buf[i + j] = temp[j];
			}
		}
	}

	fp->pos += _n;

	return _n;
}

int write(struct file *fp, int _n, unsigned char * _buf)
{
	char temp[256];
	int i, j, k;
	int BlkNum, off, BlkIdx, r, inodeLoc;
	struct inode iNode;
	int BlkSize = DATA_BLK_SIZE;

	/* read inode
	 * since there is not a root directory, fd is the index
	 * of the file entry to inode table
	 */
	inodeLoc = pfs->inodeTableStartBlk + fp->fd;
	read_ramdisk(inodeLoc, (char *)&iNode);

	BlkIdx = (int)(fp->pos / BlkSize);
	off = (int)(fp->pos % BlkSize);

	/* need to check the BlkIdx >= max len 
	 * but we don't. fix me
	 */

	if (off + _n < BlkSize) {
		if (off) {
			read_ramdisk(iNode.blkloc[BlkIdx], temp);
		} else {
			iNode.blkloc[BlkIdx] = find_datablk(pfs);
		}

		for (i = 0; i < _n; i++) {
			temp[off + i] = _buf[i];
		}
		/* write data to data blk */
		write_ramdisk(iNode.blkloc[BlkIdx], temp);
	} else {
		if (off) {
			read_ramdisk(iNode.blkloc[BlkIdx], temp);
			for (i = 0; i < BlkSize - off; i++) {
				temp[off + i] = _buf[i];
			}
			write_ramdisk(iNode.blkloc[BlkIdx], temp);
			BlkIdx++;
		} 

		r = _n - i;
		BlkNum = (int)(r / BlkSize); 
		off = (int)(r % BlkSize);
		for (j = 0; j < BlkNum; j++) {
			iNode.blkloc[BlkIdx] = find_datablk(pfs);
			/* write data to data blk */
			write_ramdisk(iNode.blkloc[BlkIdx], &_buf[i + j * BlkSize]);
			BlkIdx++;
			/* need to check the BlkIdx >= max len 
			 * but we don't. fix me
			 */
		}
		if (off) {
			for (k = 0; k < 256; k++) temp[k] = 0x00;

			for (k = 0; k < off; k++) {
				temp[k] = _buf[i + j * BlkSize + k];
			}
			iNode.blkloc[BlkIdx] = find_datablk(pfs);
			/* write data to data blk */
			write_ramdisk(iNode.blkloc[BlkIdx], temp);
		}
	}

	/* update current inode in inode table */
	iNode.fileSize += _n;
	write_ramdisk(inodeLoc, (char *)&iNode);
}

void reset(struct file *fp)
{
	fp->pos = 0;
}

void rewrite(struct file *fp)
{
	reset(fp);
	release_blks(pfs, fp->fd);
}

int eof(struct file *fp)
{
	unsigned long inodeLoc;
	struct inode iNode;

	inodeLoc = pfs->inodeTableStartBlk + fp->fd;
	read_ramdisk(inodeLoc, (char *)&iNode);

	if (fp->pos >= iNode.fileSize) return 1;
	else return 0;
}

