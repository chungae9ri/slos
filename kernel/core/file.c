#include <file.h>
#include <file_system.h>
#include <mm.h>
#include <string.h>
#include <ramdisk_io.h>

extern struct file_system *pfs;

struct file *open_file(char *str)
{
	int _fd;
	struct file *fp;
	
	fp = (struct file *)kmalloc(sizeof(struct file));

	_fd = register_file(fp);
	if (_fd < 0) {
		kfree((uint32_t)fp);
		return NULL;
	}

	fp->pfs = pfs;
	fp->fd = _fd;
	fp->pos = 0;
	strcpy(fp->name, str);
	file_system_create_file(fp);

	
	return fp;
}

int close_file(struct file *fp)
{
	unregister_file(fp);
	delete_file(fp);
	release_blks(fp);
	kfree((uint32_t)fp);
	return 0;
}

int read(struct file *fp, int _n, char *_buf)
{
	char temp[DATA_BLK_SIZE], temp2[DATA_BLK_SIZE];
	int i, j, first_entry, sec_entries, sec_entry;
	int inodeLoc, BlkNum, BlkNum2, off, r;
	struct inode iNode;
	int BlkSize = DATA_BLK_SIZE;

	sec_entries = (int)(DATA_BLK_SIZE / sizeof(int)); /* must be 64 */

	/* read inode
	 * since there is not a root directory, fd is the index
	 * of the file entry to inode table
	 */
	inodeLoc = pfs->inodeTableStartBlk + fp->fd;
	read_ramdisk(inodeLoc, (char *)(&iNode));

	if (iNode.file_size <= _n + fp->pos) _n = iNode.file_size - fp->pos;

	first_entry = (int)(fp->pos / (BlkSize * sec_entries));
	sec_entry = (int)(fp->pos / BlkSize) % sec_entries;
	off = (int)(fp->pos % BlkSize); 

	BlkNum = iNode.blkloc[first_entry];
	read_ramdisk(BlkNum, temp);

	if (off + _n < BlkSize) {
		BlkNum2 = temp[sec_entry];
		read_ramdisk(BlkNum2, temp2);
		for(i = 0; i < _n; i++) {
			_buf[i] = temp2[off + i];
		}
	} else {
		BlkNum2 = temp[sec_entry];
		read_ramdisk(BlkNum2, temp2);
		for (i = 0; i < BlkSize - off; i++) {
			_buf[i] = temp2[off + i];
		}
		r = _n - i;
		while (r) {
			sec_entry++;
			if (sec_entry == sec_entries) {
				first_entry++;
				sec_entry = 0;
			}

			if (r < BlkSize) {
				BlkNum2 = temp[sec_entry];
				read_ramdisk(BlkNum2,temp2); 
				for (j = 0; j < r; j++) {
					_buf[i + j] = temp2[j];
				}
				r = 0;
			} else {
				BlkNum2 = temp[sec_entry];
				read_ramdisk(BlkNum2,&_buf[i]); 
				i += BlkSize;
				r -= BlkSize;
			}
		}
	}

	fp->pos += _n;

	return _n;
}

int write(struct file *fp, int _n, char * _buf)
{
	char temp[DATA_BLK_SIZE], temp2[DATA_BLK_SIZE];
	int i, j, first_entry, sec_entries, sec_entry;
	int inodeLoc, BlkNum, BlkNum2, off, r;
	struct inode iNode;
	int BlkSize = DATA_BLK_SIZE;

	sec_entries = (int)(DATA_BLK_SIZE / sizeof(int)); /* must be 64 */

	/* read inode
	 * since there is not a root directory, fd is the index
	 * of the file entry to inode table
	 */
	inodeLoc = pfs->inodeTableStartBlk + fp->fd;
	read_ramdisk(inodeLoc, (char *)&iNode);

	if (iNode.file_size + _n >= inodeBlkMax * sec_entries * DATA_BLK_SIZE)
		_n = inodeBlkMax * sec_entries * DATA_BLK_SIZE - iNode.file_size;

	if (_n == 0) return 0;

	first_entry = (int)(iNode.file_size / (BlkSize * sec_entries));
	sec_entry = (int)(iNode.file_size / BlkSize) % sec_entries;
	off = (int)(iNode.file_size % BlkSize); 

	BlkNum = iNode.blkloc[first_entry];
	if (BlkNum == 0) {
		BlkNum = find_datablk();
		iNode.blkloc[first_entry] = BlkNum;
	}
	read_ramdisk(BlkNum, temp);

	if (off + _n < BlkSize) {
		if (off) {
			/*read_ramdisk(iNode.blkloc[first_entry], temp);*/
			BlkNum2 = temp[sec_entry];
			read_ramdisk(BlkNum2, temp2);
		} else {
			/*read_ramdisk(iNode.blkloc[first_entry], temp);*/
			BlkNum2 = temp[sec_entry] = find_datablk();
			write_ramdisk(iNode.blkloc[first_entry], temp);
			read_ramdisk(BlkNum2, temp2);
		}

		for (i = 0; i < _n; i++) {
			temp2[off + i] = _buf[i];
		}
		/* write data to data blk */
		write_ramdisk(BlkNum2, temp2);
	} else {
		if (off == 0) {
			temp[sec_entry] = find_datablk();
			write_ramdisk(iNode.blkloc[first_entry], temp);
		}
		/*if (off) {*/
			/*read_ramdisk(iNode.blkloc[first_entry], temp);*/
			BlkNum2 = temp[sec_entry];
			read_ramdisk(BlkNum2, temp2);
			for (i = 0; i < BlkSize - off; i++) {
				temp2[off + i] = _buf[i];
			}
			write_ramdisk(BlkNum2, temp2);
		/*} */

		r = _n - i;
		while (r) {
			sec_entry++;
			if (sec_entry == sec_entries) {
				first_entry++;
				iNode.blkloc[first_entry] = find_datablk();
				sec_entry = 0;
			}
			read_ramdisk(iNode.blkloc[first_entry], temp);
			BlkNum2 = temp[sec_entry] = find_datablk();
			write_ramdisk(iNode.blkloc[first_entry], temp);

			if (r < BlkSize) {
				for (j = 0; j < r; j++) {
					temp2[j] = _buf[i + j];
				}
				write_ramdisk(BlkNum2, temp2);
				r = 0;
			} else {
				for (j = 0; j < BlkSize; j++) {
					temp2[j] = _buf[i + j];
				}
				write_ramdisk(BlkNum2, temp2);
				r -= BlkSize;
				i += BlkSize;
			}
		}
	}

	/* update current inode in inode table */
	iNode.file_size += _n;
	write_ramdisk(inodeLoc, (char *)&iNode);
	return _n;
}

void reset(struct file *fp)
{
	fp->pos = 0;
}

void rewrite(struct file *fp)
{
	reset(fp);
	release_blks(fp);
}

int eof(struct file *fp)
{
	unsigned long inodeLoc;
	struct inode iNode;

	inodeLoc = pfs->inodeTableStartBlk + fp->fd;
	read_ramdisk(inodeLoc, (char *)&iNode);

	if (fp->pos >= iNode.file_size) return 1;
	else return 0;
}

