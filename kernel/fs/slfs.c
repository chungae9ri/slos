
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <ramdisk_io.h>
#include <slfs.h>

#define SLFS_MAGIC_STR_LEN		16
#define FNAME_LEN			32

typedef struct {
	uint32_t inode_idx;
	uint32_t update_cnt;
	uint32_t file_size;
	uint32_t datablk_addr;
	uint32_t file_id;
	uint32_t reserved;
	char name[FNAME_LEN];
} slfs_inode_t;

typedef struct {
	char magic_string[SLFS_MAGIC_STR_LEN];
	uint32_t superblk_start;
	uint32_t inode_table_bmp_start;
	uint32_t datablk_bmp_start;
	uint32_t inode_table_start;
	uint32_t datablk_start;
	uint32_t storage_size;
	uint32_t free_size;
	uint32_t file_cnt;
	uint32_t blk_size;
	bool bmounted;
} slfs_filesystem_t;

#define SLFS_SUPER_BLK_START		RAMDISK_START
#define SLFS_PAGE_SIZE_SHIFT		RAMDISK_PAGE_SIZE_SHIFT
#define SLFS_STORAGE_SIZE		RAMDISK_SIZE
#define SLFS_PAGE_SIZE			RAMDISK_PAGE_SIZE
#define SLFS_PAGE_CNT			RAMDISK_PAGE_NUM
#define MINUS_ONE			(0xFFFFFFFF)
#define WD_MASK				(0xFFFFFFFCU)
#define DWD_MASK			(0xFFFFFFF8U)
#define OFFSET_OF(st, m)		(uint32_t)(&(((st *)0)->m))

/* swap page */
#define SLFS_SWAP_PAGE_IDX		(RAMDISK_PAGE_NUM - 1)
#define SLFS_SWAP_PAGE_START		(SLFS_SWAP_PAGE_IDX << SLFS_PAGE_SIZE_SHIFT)
#define SLFS_SWAP_PAGE_SIZE		RAMDISK_PAGE_SIZE
#define SLFS_SWAP_PAGE_TAIL_SIZE	(4)
#define SLFS_SWAP_PAGE_TAIL_START	(SLFS_SWAP_PAGE_START + SLFS_SWAP_PAGE_SIZE - SLFS_SWAP_PAGE_TAIL_SIZE)
/* metadata page */
#define SLFS_METADATA_PAGE_IDX		(0x0U)
#define SLFS_METADATA_START		(0x0U)
#define SLFS_METADATA_SIZE		RAMDISK_PAGE_SIZE
/* superblk */
#define SLFS_SUPERBLK_START		(0x0U)
#define SLFS_SUPERBLK_SIZE		(0x40U)
/* inode bitmap */
#define SLFS_INODE_BITMAP_START		SLFS_SUPERBLK_SIZE
#define SLFS_INODE_BITMAP_SIZE		(0x40U)
/* datablk bitmap */
#define SLFS_DATABLK_BITMAP_START	(SLFS_INODE_BITMAP_START + SLFS_INODE_BITMAP_SIZE)
#define SLFS_DATABLK_BITMAP_SIZE	(0x40U)
/* inode table */
#define SLFS_INODE_MAX_CNT		(100U)
#define SLFS_INODE_SIZE			(sizeof(slfs_inode_t))
#define SLFS_INODE_TAB_START		(SLFS_DATABLK_BITMAP_START + SLFS_DATABLK_BITMAP_SIZE)
#define SLFS_INODE_TAB_SIZE		(SLFS_INODE_MAX_CNT * SLFS_INODE_SIZE)
/* garbage page table */
#define SLFS_GARBAGE_PAGE_TAB_START	(SLFS_INODE_TAB_START + SLFS_INODE_TAB_SIZE)
#define SLFS_GARBAGE_PAGE_TAB_SIZE	(RAMDISK_PAGE_SIZE - SLFS_INODE_TAB_START)
#define SLFS_GARBAGE_PAGE_TAB_MAX_CNT	(SLFS_GARBAGE_PAGE_TAB_SIZE >> 3)
/* datablk */
#define SLFS_DATABLK_START		RAMDISK_PAGE_SIZE
#define SLFS_DATABLK_SIZE		RAMDISK_BLK_SIZE
#define SLFS_DATABLK_MASK		(SLFS_DATABLK_SIZE - 1)
#define SLFS_DATABLK_SIZE_SHIFT		RAMDISK_BLK_SIZE_SHIFT
#define SLFS_DATABLK_MAX_CNT		((RAMDISK_SIZE - SLFS_METADATA_SIZE) >> RAMDISK_BLK_SIZE_SHIFT)
#define SLFS_DATABLK_TAIL_SIZE		(8U)
#define SLFS_DATABLK_PAYLOAD_SIZE	(RAMDISK_BLK_SIZE - SLFS_DATABLK_TAIL_SIZE)
#define SLFS_DATABLK_TAIL_HI		(SLFS_DATABLK_PAYLOAD_SIZE)
#define SLFS_DATABLK_TAIL_LOW		(SLFS_DATABLK_PAYLOAD_SIZE + 4)

#define PAGEIDX(db)			(db >> 12)
#define PAGEADDR(db)			(db & 0xFFFFE000)

#define NO_ERR				0
#define IO_ERR				1
#define METADATA_ERR			2
#define NO_DATABLK_ERR			3
#define NO_INODE_ERR			4
#define PARAM_ERR			5
#define FILE_PARAM_ERR			6
#define NULL_PTR_ERR			7
#define INODE_LAST_ERR			8

static uint8_t page_load_data[RAMDISK_PAGE_SIZE];
static slfs_filesystem_t slfs_inst;
static uint8_t superblk[SLFS_DATABLK_SIZE];
static const char magic_string[SLFS_MAGIC_STR_LEN] = "slfs filesystem";

/* COW (Copy On Write)
 * open event flag: file size, datablk_addr
 * close evet flag: inode_idx, update_cnt
 */

static int load_superblk(void)
{
	uint32_t i;
	uint32_t *pos = NULL;

	if (!io_ops.read_blk(0, superblk))
		return -IO_ERR;

	for (i = 0; i < SLFS_MAGIC_STR_LEN; i++) {
		if (superblk[i] != magic_string[i])
			return -METADATA_ERR;

		slfs_inst.magic_string[i] = superblk[i];
	}

	pos = (uint32_t *)&superblk[SLFS_MAGIC_STR_LEN];
	slfs_inst.superblk_start = *pos++;
	slfs_inst.inode_table_bmp_start = *pos++;
	slfs_inst.datablk_bmp_start = *pos++;
	slfs_inst.inode_table_start = *pos++;
	slfs_inst.datablk_start = *pos++;
	slfs_inst.storage_size = *pos++;
	slfs_inst.free_size = *pos++;
	slfs_inst.file_cnt = *pos++;
	slfs_inst.blk_size = RAMDISK_BLK_SIZE;
	slfs_inst.bmounted = true;

	return NO_ERR;
}

static int init_metadata(void)
{
	uint32_t i;
	uint32_t *pos;
	uint8_t *str;
	/* let's reuse superblk */
	str = superblk;
	for (i = 0; i < SLFS_MAGIC_STR_LEN; i++) {
		str[i] = magic_string[i];
	}

	/* Superblock start address */
	pos = (uint32_t *)(&superblk[SLFS_MAGIC_STR_LEN]);
	*pos++ = SLFS_SUPER_BLK_START;
	/* iNode bitmap start address */
	*pos++ = SLFS_INODE_BITMAP_START;
	/* Datablock bitmap start address */
	*pos++ = SLFS_DATABLK_BITMAP_START;
	/* iNode table start address */
	*pos++ = SLFS_INODE_TAB_START;
	/* Datablock start address */
	*pos++ = SLFS_DATABLK_START;
	/* Data storage total size in bytes */
	*pos++ = SLFS_DATABLK_PAYLOAD_SIZE * SLFS_DATABLK_MAX_CNT;
	/* Data storage free size in bytes */
	*pos++ = SLFS_DATABLK_PAYLOAD_SIZE * SLFS_DATABLK_MAX_CNT;
	/* File count */
	*pos = 0U;

	for (i = 0; i < RAMDISK_PAGE_SIZE / RAMDISK_BLK_SIZE; i++)
		if (!io_ops.write_blk(i, &superblk[i * RAMDISK_BLK_SIZE]))
			return -IO_ERR;

	return NO_ERR;
}

static int copy_flash_to_flash(uint32_t src_addr, uint32_t dst_addr, uint32_t len)
{
	uint8_t buf[SLFS_DATABLK_SIZE];
	uint32_t copy_len;
	uint32_t offset;

	/* 1. copy flash region to other flash region */
	offset = 0;
	while (len > 0) {
		if (len > SLFS_DATABLK_SIZE)
			copy_len = SLFS_DATABLK_SIZE;
		else
			copy_len = len;

		if (io_ops.read(src_addr + offset, copy_len, buf))
			return -IO_ERR;

		if (io_ops.write(dst_addr + offset, copy_len, buf))
			return -IO_ERR;

		len -= copy_len;
		offset += copy_len;
	}

	return NO_ERR;
}

static int rw_page(uint32_t page_idx, bool bwrite)
{
	uint32_t i;
	uint32_t addr;
	uint32_t len;
	uint32_t garbage_page_idx;
	uint8_t tail_buf[SLFS_DATABLK_TAIL_SIZE];
	int ret;

	if (SLFS_PAGE_CNT <= page_idx)
		return -PARAM_ERR;

	if (!bwrite)  {
		addr = page_idx << SLFS_PAGE_SIZE_SHIFT;
		io_ops.read(addr, SLFS_PAGE_SIZE, page_load_data);

	}
	else {
		/* 1. erase temp page */
		if (io_ops.erase_page(SLFS_SWAP_PAGE_IDX))
			return -IO_ERR;

		/* 2. write back to the temp page */
		if (page_idx == 0) { /* if page_idx is metadata page */
			addr = SLFS_SWAP_PAGE_START;
			len = SLFS_PAGE_SIZE - SLFS_DATABLK_TAIL_SIZE;
		}
		else {
			addr = SLFS_SWAP_PAGE_START;
			len = SLFS_PAGE_SIZE;
		}
		if (io_ops.write(addr, len, page_load_data))
			return -IO_ERR;

		/* 3. Record target page idx.
		 *    if page_idx is metadata page, write it to the
		 *    last 8 byte of the temp page.
		 *    if page_idx is one of data pages, write it to the
		 *    metadata garbage page section.
		 *    this is the closure of writing to temp page and
		 *    the temp page can be safely used to recover the 
		 *    original page in mount()
		 */
		if (page_idx == 0) {
			addr = SLFS_SWAP_PAGE_START + SLFS_PAGE_SIZE - SLFS_DATABLK_TAIL_SIZE;
			len = SLFS_DATABLK_TAIL_SIZE;
			((uint32_t *)tail_buf)[0] = page_idx << SLFS_PAGE_SIZE_SHIFT;
			((uint32_t *)tail_buf)[1] = MINUS_ONE;
			if (io_ops.write(addr, len, tail_buf)) 
				return -IO_ERR;	
		} else {
			for (i = 0; i < SLFS_GARBAGE_PAGE_TAB_MAX_CNT; i++) {
				garbage_page_idx = *((uint32_t*)(SLFS_GARBAGE_PAGE_TAB_START + (i * sizeof(uint64_t))));
				if (garbage_page_idx == MINUS_ONE) {
					((uint32_t *)tail_buf)[0] = page_idx << SLFS_PAGE_SIZE_SHIFT;
					((uint32_t *)tail_buf)[1] = MINUS_ONE;
					addr = SLFS_GARBAGE_PAGE_TAB_START + (i * sizeof(uint64_t));
					len = SLFS_DATABLK_TAIL_SIZE;
					if (io_ops.write(addr, len, tail_buf))
						return -IO_ERR;
					break;
				}
			}
		}
		/* 4. Erase the target page */
		if (io_ops.erase_page(page_idx))
			return -IO_ERR;
		/* 5. copy it from swap page to target page */
		ret = copy_flash_to_flash(SLFS_SWAP_PAGE_START, 
					 (page_idx << SLFS_PAGE_SIZE_SHIFT),
					 SLFS_PAGE_SIZE);
		if (ret)
			return ret;
		/* 6. erase swap page */
		if (io_ops.erase_page(SLFS_SWAP_PAGE_IDX))
			return -IO_ERR;

	}

	return NO_ERR;
}

static int update_fp(slfs_file_t *pf)
{
	int ret;
	uint32_t i;
	slfs_inode_t *pinode;

	/* 1. load the metadata page */
	ret = rw_page(SLFS_METADATA_PAGE_IDX, false);

	if (ret)
		return ret;

	pinode = (slfs_inode_t *)(&page_load_data[SLFS_INODE_TAB_START]);
	/* 2. for all inode in the metadata, if it is 
	 *    a free file index, do allocate it to the 
	 *    new file descriptor 
	 *    FileSize and DataBlkAddr is the record of CoW(Copy on Write)
	 *    If one of these isn't MINUS_ONE, that means 
	 *    the file with that iNode should be one of
	 *    dangled file, oudated file, deleted file or
	 *    a valid file. 
	 */
	for (i = 0; i < SLFS_INODE_MAX_CNT; i++) {
		if (pinode->datablk_addr == MINUS_ONE) {
			pf->inode_idx = i;
			pf->update_cnt++;
			/* if pf->fd isn't MINUS_ONE, then current fd is 
			 * kept through all subsequent allocation
			 */
			if (pf->fd == MINUS_ONE)
				pf->fd = i;
			break;
		}
		pinode++;
	}

	if (i == SLFS_INODE_MAX_CNT)
		return -NO_INODE_ERR;

	return NO_ERR;
}

static int alloc_datablk(slfs_file_t *pf, uint32_t len)
{
	/* 1. calculate blks needed based on fp->pos and BytesWrite.
	 * 2. for all datablks check the tail 8bytes of current datablk, 
	 *    if it shows a free blk (0xFFFFFFFF 0xFFFFFFFF), update the
	 *    datablk linked list (prev, next) until the allocated
	 *    datablk reached blk required
	 */
	uint32_t i;
	uint32_t blks_needed;
	uint32_t blks_alloced;
	uint32_t datablk_addr;
	uint32_t prev_datablk_addr;
	uint32_t prev_prev_datablk_addr;
	uint32_t next_datablk_addr;
	uint32_t offset;
	uint32_t addr;

	if (!pf)
		return -NULL_PTR_ERR;

	if ((pf->pos + len) > pf->file_size)
		pf->file_size = pf->pos + len;
	
	len = pf->file_size;
	blks_needed = 0U;

	while (0U < len) {
		blks_needed++;
		if (SLFS_DATABLK_PAYLOAD_SIZE < len)
			len -= SLFS_DATABLK_PAYLOAD_SIZE;
		else
			break;
	}

	datablk_addr = SLFS_DATABLK_START;
	blks_alloced = 0;
	for (i = 0; i < SLFS_DATABLK_MAX_CNT; i++) {
		if (io_ops.read(datablk_addr + SLFS_DATABLK_TAIL_HI, sizeof(uint32_t), (uint8_t *)&prev_datablk_addr))
			return -IO_ERR;

		if (io_ops.read(datablk_addr + SLFS_DATABLK_TAIL_LOW, sizeof(uint32_t), (uint8_t *)&next_datablk_addr))
			return -IO_ERR;

		if ((MINUS_ONE == prev_datablk_addr) && (MINUS_ONE == next_datablk_addr)) {
			/* writing the file size and the first datablk addr into the inode
			 * this ia an OPEN event of COW.
			 */
			if (blks_alloced == 0U) {
				pf->datablk_addr = datablk_addr;
				offset = OFFSET_OF(slfs_inode_t, file_size);
				addr = SLFS_INODE_TAB_START + (pf->inode_idx * SLFS_INODE_SIZE) + offset;
				if (io_ops.write(addr, sizeof(uint32_t), (uint8_t *)&pf->file_size))
					return -IO_ERR;

				offset = OFFSET_OF(slfs_inode_t, datablk_addr);
				addr = SLFS_INODE_TAB_START + (pf->inode_idx * SLFS_INODE_SIZE) + offset;
				if (io_ops.write(addr, sizeof(uint32_t), (uint8_t *)&datablk_addr))
					return -IO_ERR;

				prev_prev_datablk_addr = SLFS_INODE_TAB_START + (pf->inode_idx * SLFS_INODE_SIZE);
				prev_datablk_addr = datablk_addr;
			} else {
				/* update prev datablk addr ptr */
				addr = prev_datablk_addr + SLFS_DATABLK_TAIL_HI;
				if (io_ops.write(addr, sizeof(uint32_t), (uint8_t *)&prev_prev_datablk_addr))
					return -IO_ERR;

				addr = prev_datablk_addr + SLFS_DATABLK_TAIL_LOW;
				if (io_ops.write(addr, sizeof(uint32_t), (uint8_t *)&datablk_addr))
					return -IO_ERR;

				prev_prev_datablk_addr = prev_datablk_addr;
				prev_datablk_addr = datablk_addr;
			}
			blks_alloced++;
		}
		if (blks_alloced == blks_needed) {
			pf->allocedblk_num = blks_alloced;
			/* update the last datablk addr ptrs */
			addr = datablk_addr + SLFS_DATABLK_TAIL_HI;
			if (io_ops.write(addr, sizeof(uint32_t), (uint8_t *)&prev_prev_datablk_addr))
					return -IO_ERR;

			addr = datablk_addr + SLFS_DATABLK_TAIL_LOW;
			prev_datablk_addr = 0;
			if (io_ops.write(addr, sizeof(uint32_t), (uint8_t *)&prev_datablk_addr))
					return -IO_ERR;
		}
		datablk_addr += SLFS_DATABLK_SIZE;
	}

	if ((i == SLFS_DATABLK_MAX_CNT) && (blks_alloced != blks_needed))
		return -NO_DATABLK_ERR;

	return NO_ERR;
}

static uint32_t traverse_datablk_list(slfs_file_t *pf,
		uint32_t datablk_idx,
		uint32_t *pnew_datablk_idx)
{
	/* 1. traverse the list of current file pf.
	 *    return the last datablk address
	 */
	uint32_t next_datablk_addr;
	uint32_t cur_datablk_addr;

	if (!pf) {
		cur_datablk_addr = MINUS_ONE;
		if (pnew_datablk_idx)
			*pnew_datablk_idx = MINUS_ONE;
	} else if (MINUS_ONE == pf->datablk_addr) {
		if (pnew_datablk_idx)
			*pnew_datablk_idx = MINUS_ONE;

		next_datablk_addr = MINUS_ONE;
		cur_datablk_addr = MINUS_ONE;
	} else {
		if (pnew_datablk_idx)
			*pnew_datablk_idx = 0U;

		cur_datablk_addr = pf->datablk_addr;
		if (!io_ops.read(cur_datablk_addr + SLFS_DATABLK_TAIL_LOW, 
					sizeof(uint32_t), 
					(uint8_t *)&next_datablk_addr)) 
			return MINUS_ONE;

		while ((0U != datablk_idx) && (MINUS_ONE != next_datablk_addr)) {
			cur_datablk_addr = next_datablk_addr;
			if (!io_ops.read(cur_datablk_addr + SLFS_DATABLK_TAIL_LOW,
						sizeof(uint32_t),
						(uint8_t *)&next_datablk_addr))
				return MINUS_ONE;

			datablk_idx--;

			if (pnew_datablk_idx)
				(*pnew_datablk_idx)++;
		}
	}

	return cur_datablk_addr;
}



static int delete_datablks(slfs_inode_t *pinode)
{
	/* 1. go to the last datablk  
	 * 2. traverse the datablk list from the last datablk,
	 *    erase it 
	 */
	uint32_t *pdatablk;
	uint32_t i;
	uint32_t datablk_addr;
	uint32_t cur_datablk_addr;
	uint32_t cur_page_idx;

	datablk_addr = pinode->datablk_addr;
	/* go to the last datablk */
	while (MINUS_ONE != datablk_addr) {
		cur_datablk_addr = datablk_addr;
		datablk_addr = *((uint32_t *)(cur_datablk_addr + SLFS_DATABLK_TAIL_LOW));
		/* next datablk is dangled, or was already deleted.
		 * this is happening when powered off during delete_datablks().
		 */
		if (cur_datablk_addr != *((uint32_t *)(datablk_addr + SLFS_DATABLK_TAIL_HI)))
			break;
	}

	do {
		cur_page_idx = PAGEIDX(cur_datablk_addr);
		if (rw_page(cur_page_idx, false))
			return -IO_ERR;

		do {
			pdatablk = (uint32_t *)&page_load_data[cur_datablk_addr - PAGEADDR(cur_datablk_addr)];
			cur_datablk_addr = *((uint32_t *)(cur_datablk_addr + SLFS_DATABLK_TAIL_HI));
			for (i = 0; i < (SLFS_DATABLK_SIZE >> 4); i++) 
				pdatablk[i] = MINUS_ONE;

			if (cur_datablk_addr == 0U)
				break;
		} while (cur_page_idx == PAGEIDX(cur_datablk_addr));

		if (rw_page(cur_page_idx, true))
			return -IO_ERR;
	} while (SLFS_DATABLK_START <= cur_datablk_addr);

	return NO_ERR;
}

static int recover_page(uint32_t page_idx)
{
	/* 1. copy swap page into target page
	 * 2. if metadata page idx, just clean up the swap page.
	 *    if data page idx, clean the garbage page section
	 *    in the metadata page and clean the swap page
	 */
	uint32_t i;
	uint32_t *pdata;
	int ret;

	if (io_ops.erase_page(SLFS_METADATA_PAGE_IDX))
		return -IO_ERR;

	ret = copy_flash_to_flash(SLFS_SWAP_PAGE_START, 
				 SLFS_METADATA_START, 
				 SLFS_PAGE_SIZE);
	if (ret)
		return ret;

	if (page_idx != SLFS_METADATA_PAGE_IDX) {
		if (rw_page(SLFS_METADATA_PAGE_IDX, false))
			return -IO_ERR;
		for (i = 0; i < SLFS_GARBAGE_PAGE_TAB_MAX_CNT; i++) {
			pdata = (uint32_t *)&page_load_data[SLFS_GARBAGE_PAGE_TAB_START + (i * sizeof(uint64_t))];
			*pdata = MINUS_ONE;
			pdata++;
			*pdata = MINUS_ONE;
		}
		if (rw_page(SLFS_METADATA_PAGE_IDX, true))
			return -IO_ERR;
	} else {
		if(io_ops.erase_page(SLFS_SWAP_PAGE_IDX))
			return -IO_ERR;
	}

	return NO_ERR;
}

static int slfs_memcpy(void *pdst, void *psrc, uint32_t len)
{
	uint32_t i = 0;

	if ((NULL != psrc) && (NULL != pdst)) {
		for (i = 0; i < len; i++) 
			((uint8_t *)pdst)[i] = ((uint8_t *)psrc)[i];

		return NO_ERR;
	} else 
		return -IO_ERR;
}

static int collect_garbage(void)
{
	/* 1. collect obsolete iNodes and
	 *    delete datablks from dangled iNode
	 * 2. Record the deleted file's iNode
	 * 3. delete datablks of deleted file
	 * 4. Starting update of the metadata page
	 * 4-1. Clean the garbage page idx section
	 * 4-2. Delete the dangled iNode
	 * 4-3. Delete the deleted files' iNode
	 * 4-1. Update metadata page 
	 */
	uint32_t i;
	uint32_t j;
	uint32_t k;
	uint32_t del_file_desc[SLFS_INODE_MAX_CNT * 2];
	uint32_t deleted_fd_idx = 0;
	uint32_t file_id;
	uint32_t update_cnt;
	slfs_inode_t *pinode;
	slfs_inode_t inode;

	for (i = 0; i < SLFS_INODE_MAX_CNT * 2; i++) 
		del_file_desc[i] = MINUS_ONE;

	/* 1. collect obsolete inodes */
	for (i = 0; i < SLFS_INODE_MAX_CNT; i++) {
		if (io_ops.read(SLFS_INODE_TAB_START + i * SLFS_INODE_SIZE, SLFS_INODE_SIZE, (uint8_t *)&inode))
			return -IO_ERR;

		if ((inode.file_id == MINUS_ONE) && (inode.file_size == MINUS_ONE))
			continue;

		/* 2. delete datablks from dangled inode */
		if ((inode.inode_idx == MINUS_ONE) && (inode.update_cnt == MINUS_ONE))
			delete_datablks(&inode);
		else if ((inode.file_size == 0) && (inode.datablk_addr == 0)) {
			del_file_desc[deleted_fd_idx++] = inode.file_id;
			del_file_desc[deleted_fd_idx++] = inode.update_cnt;
		}
	}
	deleted_fd_idx = 0;
	/* 3. delete datablks of deleted file */
	for (i = 0; i < SLFS_INODE_MAX_CNT; i++) {
		file_id = del_file_desc[deleted_fd_idx++];
		update_cnt = del_file_desc[deleted_fd_idx++];
		if (file_id == MINUS_ONE)
			break;

		for (j = 0; j < SLFS_INODE_MAX_CNT; j++) {
			if (!io_ops.read(SLFS_INODE_TAB_START + j * SLFS_INODE_SIZE, SLFS_INODE_SIZE, (uint8_t *)&inode))
				return -IO_ERR;

			if ((inode.file_id == file_id) && (inode.update_cnt == update_cnt))
				delete_datablks(&inode);
		}
	}
	
	/* TODO: if powered-off here, there are incorrect garbage page
	 *       index in the metadata and it will overwrite the right
	 *       data with erased data in recovery() process.
	 */
	/* 4. starting update of the metadata page */
	if (rw_page(SLFS_METADATA_PAGE_IDX, false))
		return -IO_ERR;

	/* 5. clean the garbage page idx section */
	for (i = 0; i < (SLFS_GARBAGE_PAGE_TAB_SIZE >> 2); i++) 
		((uint32_t *)page_load_data)[(SLFS_GARBAGE_PAGE_TAB_START >> 2) + i] = MINUS_ONE;

	for (i = 0; i < SLFS_INODE_MAX_CNT; i++) {
		pinode = (slfs_inode_t *)(&page_load_data[SLFS_INODE_TAB_START + (i * SLFS_INODE_SIZE)]);
		/* 6. delete the dangled inode */
		if ((pinode->inode_idx == MINUS_ONE) && (pinode->update_cnt == MINUS_ONE))
			for (j = 0; j < (SLFS_INODE_SIZE >> 2); j++)
				((uint32_t *)pinode)[j] = MINUS_ONE;

		/* 7. delete the deleted files' inode */
		for (j = 0; j < (SLFS_INODE_MAX_CNT * 2); j+= 2) {
			file_id = del_file_desc[j];
			update_cnt = del_file_desc[j + 1];
			if (file_id == MINUS_ONE)
				break;

			/* 7-1. found deleted file's inode and delete it */
			if ((pinode->file_id == file_id) && (pinode->update_cnt == update_cnt)) {
				for (k = 0; k < (SLFS_INODE_SIZE >> 2); k++)
					((uint32_t *)pinode)[k] = MINUS_ONE;

				break;
			}
		}
	}

	/* 8. update metadata page */
	if (rw_page(SLFS_METADATA_PAGE_IDX, true))
		return -IO_ERR;

	return NO_ERR;
}


int slfs_mount(void)
{
	uint8_t magic_str_buf[SLFS_MAGIC_STR_LEN];
	uint8_t tail_buf[SLFS_DATABLK_TAIL_SIZE] = {0, };
	uint32_t i;
	uint32_t garbage_page_idx;
	uint32_t garbage_page_idx_prev;
	int ret;

	/* 1. check the swap page if there is a valid data in the swap page */
	if (io_ops.read(SLFS_SWAP_PAGE_TAIL_START, SLFS_SWAP_PAGE_TAIL_SIZE, tail_buf))
		return -IO_ERR;

	/* 1-1. metadata page update wasn't finished */
	if (((uint32_t *)tail_buf)[0] == SLFS_METADATA_PAGE_IDX) {
		ret = recover_page(SLFS_METADATA_PAGE_IDX);
		if (ret)
			return ret;
	}
	/* 1-2. check if there is any pending page update from garbage page table in the metadata.
	 *      only the last entry is the right page index that was stopped while being udpated. 
	 */
	else {
		for (i = 0; i < SLFS_GARBAGE_PAGE_TAB_MAX_CNT; i++) {
			garbage_page_idx = *((uint32_t *)(SLFS_GARBAGE_PAGE_TAB_START + (i * sizeof(uint32_t))));
			if (MINUS_ONE == garbage_page_idx)
				break;
			else 
				garbage_page_idx_prev = garbage_page_idx;
		}
		if (garbage_page_idx_prev != 0) {
			ret = recover_page(garbage_page_idx_prev);
			if (ret)
				return ret;
		}
	}
	/* 2. load the superblk */
	if (io_ops.read(SLFS_SUPERBLK_START, SLFS_MAGIC_STR_LEN, magic_str_buf))
		return -IO_ERR;

	for (i = 0; i < SLFS_MAGIC_STR_LEN; i++)
		if (magic_str_buf[i] != magic_string[i])
			break;
	/* Be cautious: if the flash storage doesn't have the right SLFS signature string, it will be formatted. */
	if (i != SLFS_MAGIC_STR_LEN) {
		/* 2-1. format if it doesn't have a magic string */
		ret = slfs_format();
		if (ret)
			return ret;
		/* 2-2. init the metadata */
		ret = init_metadata();
		if (ret)
			return ret;
	}
	/* 2-3. load superblk */
	ret = load_superblk();
	if (ret)
		return ret;

	return NO_ERR;
}

int slfs_umount(void)
{
	slfs_inst.bmounted = false;

	return NO_ERR;
}

int slfs_format(void)
{
	/* File system format does erase all contents in 
	 * the metadata page (superblock, inode table, etc)
	 * and all datablk pages
	 */
	return io_ops.erase_chip();
}

int slfs_open(const uint8_t *pname, slfs_file_t *pf)
{
	bool bfound;
	bool bfirst_found;
	uint32_t i;
	uint32_t j;
	slfs_inode_t inode;
	slfs_inode_t inode_last;
	slfs_inode_t inode_first;

	/* 1. check if there is already a file descriptor in the inode table,
	 *    then record the first and latest inode having the same file name.
	 *    the update_cnt can be considered as a timestamp of the inode
	 */
	for (i = 0; i < SLFS_INODE_MAX_CNT; i++) {
		if (io_ops.read(SLFS_INODE_TAB_START + i * sizeof(inode), sizeof(inode), (uint8_t *)&inode))
			return -IO_ERR;

		bfound = false;
		for (j = 0; j < SLFS_FNAME_LEN; j++) {
			if (pname[j] != inode.name[j])
				break;
			else if ('\0' == pname[j]) {
				bfound = true;
				break;
			}
		}

		bfirst_found = true;
		if (bfound) {
			if (bfirst_found) {
				bfirst_found = false;
				slfs_memcpy(&inode_first, &inode, sizeof(inode));
				slfs_memcpy(&inode_last, &inode, sizeof(inode));
			} else {
				if (inode_last.update_cnt < inode.update_cnt)
					slfs_memcpy(&inode_last, &inode, sizeof(inode));
				else 
					slfs_memcpy(&inode_first, &inode, sizeof(inode));
			}
			bfound = false;
		}
	}
	/* there is already a file created */
	if (inode_last.inode_idx != MINUS_ONE) {
		pf->fd = inode_first.file_id;
		pf->file_size = inode_last.file_size;
		pf->pos = 0U;
		pf->datablk_addr = inode_last.datablk_addr;
		pf->inode_idx = inode_last.inode_idx;
		for (i = 0; i < SLFS_FNAME_LEN; i++) {
			pf->name[i] = inode_last.name[i];
			if ('\0' == inode_last.name[i]) 
				break;
		}
		pf->update_cnt = inode_last.update_cnt;
		traverse_datablk_list(pf, MINUS_ONE, &pf->allocedblk_num);
		if (pf->allocedblk_num == MINUS_ONE)
			pf->allocedblk_num = 0;
		else
			pf->allocedblk_num++;
	} else {
		/* lazy inode allocation. 
		 * if there is no inode entry, the inode is alloced only
		 * when the file write runs because of the COW implementation.
		 */
		pf->fd = MINUS_ONE;
		pf->file_size = 0;
		pf->inode_idx = MINUS_ONE;
		pf->pos = 0;
		pf->open_cnt = 1;
		pf->datablk_addr = MINUS_ONE;
		pf->allocedblk_num = 0;
		pf->update_cnt = 0;
		for (i = 0; i < SLFS_FNAME_LEN; i++) {
			pf->name[i] = pname[i];
			if ('\0' == pname[i])
				break;
		}
	}

	return NO_ERR;
}

int slfs_seek(slfs_file_t *pf, uint32_t offset, slfs_fseek_t whence)
{
	uint32_t pos;

	if (!pf)
		return -NULL_PTR_ERR;

	if (!pf->open_cnt)
		return -FILE_PARAM_ERR;

	switch (whence) {
	case SLFS_SEEK_SET:
		pos = offset;
		if (pos > pf->file_size)
			return -FILE_PARAM_ERR;
		else 
			pf->pos = pos;
		break;
	case SLFS_SEEK_CUR:
		pos = pf->pos + offset;
		if (pos > pf->file_size)
			return -FILE_PARAM_ERR;
		else 
			pf->pos = pos;
		break;
	case SLFS_SEEK_END:
		pos = pf->file_size - offset;
		if (pos > pf->file_size)
			return -FILE_PARAM_ERR;
		else
			pf->pos = pos;
		break;
	default:
		break;
	}

	return NO_ERR;
}

int slfs_write(slfs_file_t *pf, uint8_t *pbuf, uint32_t bytes_write)
{
	/* 1. Calculate the right file size with the file Pos
	 *    back up the file name
	 *
	 * 2. Update the pf file descriptor with the 
	 *    free iNode info in the iNode table 
	 * 2-1. If there is not free iNode in the iNode table,
	 *      collect garbages and try again to found a free iNode
	 * 2-2. Try again buf if still no free iNode,
	 *      then return error
	 *
	 * 3. Allocate the datablks and connect them with doubly
	 *    linked list. The prev of the first datablk points to
	 *    the iNode, the next of the last iNode has 0.
	 *    alloc_datablk record the OPEN event of COW.
	 * 3-1. If no free datablks, run garbage collection
	 * 3-2. Try to alloc datablk again and if still not
	 *      enough datablks, error returns
	 *
	 * 4. If there is enough space to alloc iNode and datablks,
	 *    write back the fp->name to the new alloc-ed iNode
	 *
	 * 5. Write back the iNode FileId to the new iNode
	 *
	 * 6. Write the buffer data to the flash datablks
	 *    for the first time.
	 * 6-1. This is determined only with the filesize.
	 *      There is no iNode having filesize 0 in
	 *      the iNode table
	 * 6-2. Calc the padded byte data length
	 * 6-3. If current data size is still bigger than
	 *      one datablk size, flush one datablk size
	 *      buffer adata into the datablk
	 * 6-4. If current data size is less than one datablk
	 *      write it to the datablk with the right handling
	 *      of tail data
	 *
	 * 7. Update (append, modify) the prev file with fseek
	 * 7-1. Copy all datablks from previous file's datablk.
	 *      While copying, update the new contents if it falls
	 *      into datablk currently being copied
	 * 7-1-1. copy prev file datablk content to gDatablk
	 * 7-1-2. update the content if the pos or pos + len is 
	 *        in current datablk
	 * 7-1-3. write only payload data from gDatablk back 
	 *        to the new file desc's datablks.
	 *        The prev, next address in the new datablks were
	 *        already updated in the alloc_datablk().
	 * 7-1-4. Move to the next datablk in each prev
	 *        file and current file
	 * 7-2. If there are stil bytes to be written (file append) 
	 *
	 * 8. Update the validity of iNode with iNodeIdx and UpdateCntN.
	 *    This is the record of CLOSE event of COW
	 */
	uint8_t tail_buf[SLFS_DATABLK_TAIL_SIZE];
	uint8_t inode_name[SLFS_FNAME_LEN];
	uint8_t data_buf[SLFS_DATABLK_SIZE];
	uint32_t i;
	uint32_t j;
	uint32_t datablk_addr;
	uint32_t buf_pos;
	uint32_t padded_bytes_write;
	uint32_t pos;
	uint32_t datablk_addr_prev;
	uint32_t allocedblk_num_prev;
	uint32_t file_size_prev;
	uint32_t len;
	uint32_t offset;
	int ret;

	if (!pf)
		return -NULL_PTR_ERR;

	if ((pf->open_cnt == 0) || (pbuf == NULL) || (bytes_write == 0))
		return -FILE_PARAM_ERR;

	datablk_addr_prev = pf->datablk_addr;
	file_size_prev = pf->file_size;
	allocedblk_num_prev = pf->allocedblk_num;
	/* 1. calculate the right file size with the file pos */
	if (pf->file_size == 0)
		pf->file_size = bytes_write;
	else if (pf->pos + bytes_write > pf->file_size)
		pf->file_size = pf->pos + bytes_write;

	/* backup the file name */
	slfs_memcpy(inode_name, pf->name, SLFS_FNAME_LEN);

	/* 2. update the pf file descriptor with the 
	 *    free inode info in the inode table
	 */
	ret = update_fp(pf);
	if (ret == -NO_INODE_ERR) {
		/* 2-1. if there is not free inode in the inode table,
		 *      collect garbages and try again to find a free inode
		 */
		ret = collect_garbage();
		if (ret)
			return ret;
		/* 2-2. try again and if sill no free inode, then return error */
		ret = update_fp(pf);
		if (ret)
			return ret;
	}
	/* 3. allocate the datablks and connect them with doubly
	 *    linked list. the prev of the first datablk proints to
	 *    the inode, the next of the last inode has 0.
	 *    alloc_datablk record the OPEN event of COW.
	 */
	ret = alloc_datablk(pf, bytes_write);
	/* 3-1. if no free datablk, run garbage collection */
	if (ret == -NO_DATABLK_ERR) {
		ret = collect_garbage();
		if (ret)
			return ret; 

		/* 3-2. try to alloc datablk again and if still not enough
		 *      datablks, error returns
		 */
		ret = alloc_datablk(pf, bytes_write);
		if (ret)
			return ret;
	}
	/* 4. if there is enough space to alloc inode and datablks,
	 *    write back the fp->name to the new alloc-ed inode
	 */
	offset = OFFSET_OF(slfs_inode_t, name);
	if (io_ops.write((SLFS_INODE_TAB_START + 
			  (pf->inode_idx * SLFS_INODE_SIZE) + 
			  offset),
			  SLFS_FNAME_LEN,
			  inode_name))
		return -IO_ERR;
	/* 5. write back the inode file_id to the new inode */
	((uint32_t *)tail_buf)[0] = pf->fd;
	((uint32_t *)tail_buf)[1] = MINUS_ONE;
	offset = OFFSET_OF(slfs_inode_t, file_id);
	if (io_ops.write((SLFS_INODE_TAB_START +
			  (pf->inode_idx * SLFS_INODE_SIZE) +
			  offset),
			  SLFS_DATABLK_TAIL_SIZE,
			  tail_buf))
		return -IO_ERR;

	/* 6. write the buffer data to the flash datablks for the first time */
	/* 6-1. this is determined only with the filesize.
	 *      there is no inode having file_size 0 in 
	 *      the inode table
	 */
	if (file_size_prev == 0) {
		/* 6-2. calc the padded byte data length */
		padded_bytes_write = bytes_write;
		if ((padded_bytes_write & ~DWD_MASK) != 0) {
			padded_bytes_write &= DWD_MASK;
			padded_bytes_write += SLFS_DATABLK_TAIL_SIZE;
		}
		datablk_addr = pf->datablk_addr;
		buf_pos = 0;
		while(padded_bytes_write > 0) {
			/* 6-3. if current data size is still bigger than 
			 *      one datablk size, flush one datablk size
			 *      buffer data into the datablk
			 */
			if (padded_bytes_write > SLFS_DATABLK_PAYLOAD_SIZE) {
				if (io_ops.write(datablk_addr,
						 SLFS_DATABLK_PAYLOAD_SIZE,
						 (uint8_t *)(&pbuf[buf_pos])))
					return -IO_ERR;

				padded_bytes_write -= SLFS_DATABLK_PAYLOAD_SIZE;
				buf_pos += SLFS_DATABLK_PAYLOAD_SIZE;
				/* update the ptr to next datablk in the linked list */
				datablk_addr = *(uint32_t *)(datablk_addr + 
						SLFS_DATABLK_TAIL_LOW);
				slfs_inst.free_size -= SLFS_DATABLK_PAYLOAD_SIZE;
			}
			/* 6-4. if current data size is less than one datablk size
			 *      write it to the datablk with the right handling 
			 *      of tail data
			 */
			else {
				if (SLFS_DATABLK_TAIL_SIZE < padded_bytes_write) {
					if (io_ops.write(datablk_addr,
						          (padded_bytes_write - SLFS_DATABLK_TAIL_SIZE),
						          (uint8_t *)(&pbuf[buf_pos])))
					return -IO_ERR;
					datablk_addr += SLFS_DATABLK_TAIL_SIZE;
					buf_pos += (padded_bytes_write - SLFS_DATABLK_TAIL_SIZE);
				}
				for (i = 0; i < (bytes_write - buf_pos); i++)
					tail_buf[i] = pbuf[buf_pos + i];

				if (io_ops.write(datablk_addr, SLFS_DATABLK_TAIL_SIZE, tail_buf))
					return -IO_ERR;

				padded_bytes_write = 0;
				slfs_inst.free_size -= SLFS_DATABLK_PAYLOAD_SIZE;
			}
		}
	}
	/* 7. update (append, modify) the prev file with fseek */
	else {
		datablk_addr = pf->datablk_addr;
		pos = pf->pos;
		/* let's reuse padded_bytes_write variable for counting
		 * the bytes write. 
		 */
		padded_bytes_write = 0;
		/* 7-1. copy all datablks from previous file's datablk.
		 *      while copying, update the new contents if it falls
		 *      into datablk currently being copied
		 */
		for (i = 0; i < allocedblk_num_prev; i++) {
			/* 7-1-1. copy prev file datablk content to data buf */
			if (io_ops.read(datablk_addr_prev, SLFS_DATABLK_PAYLOAD_SIZE, data_buf))
				return -IO_ERR;

			/* 7-1.2. update the content if the pos or pos + len 
			 *        is in current datablk
			 */
			if ((bytes_write> 0) && (pos < SLFS_DATABLK_PAYLOAD_SIZE)) {
				len = SLFS_DATABLK_PAYLOAD_SIZE - pos;
				len = (len < bytes_write) ? len : bytes_write;
				for (j = 0; j < len; j++) 
					data_buf[pos + j] = pbuf[i * SLFS_DATABLK_PAYLOAD_SIZE +j];

				if (bytes_write > len)
					bytes_write -= len;
				else {
					bytes_write = 0;
					for (j = pos + j; j < SLFS_DATABLK_PAYLOAD_SIZE; j++)
						data_buf[j] = 0xFF;
				}
				pos = 0;
			} else 
				pos -= SLFS_DATABLK_PAYLOAD_SIZE;

			/* 7-1-3. write only payload data from data_buf back to
			 *        the new file desc's datablks.
			 *        the prev, next address in the new datablks were
			 *        already updated in the alloc_datablk().
			 */
			if (io_ops.write(datablk_addr, SLFS_DATABLK_PAYLOAD_SIZE, data_buf))
				return -IO_ERR;

			padded_bytes_write += len;
			/* 7-1-4. move to the next datablk in each prev 
			 *        file and current file
			 */
			datablk_addr_prev = *((uint32_t *)(datablk_addr_prev + SLFS_DATABLK_TAIL_LOW));
			datablk_addr = *((uint32_t *)(datablk_addr + SLFS_DATABLK_TAIL_LOW));
		}
		/* 7-2. if there are still bytes to be written (file append */
		if (bytes_write > 0) {
			for (i = 0; i < (pf->allocedblk_num - allocedblk_num_prev); i++) {
				if (bytes_write < SLFS_DATABLK_PAYLOAD_SIZE) {
					len = bytes_write;
					bytes_write = 0;
				} else {
					len = SLFS_DATABLK_PAYLOAD_SIZE;
					bytes_write -= SLFS_DATABLK_PAYLOAD_SIZE;
				}

				if (io_ops.write(datablk_addr, len, &pbuf[padded_bytes_write + (i * SLFS_DATABLK_PAYLOAD_SIZE)]))
					return -IO_ERR;

				datablk_addr = *((uint32_t *)(datablk_addr + SLFS_DATABLK_TAIL_LOW));
			}
		}
	}
	/* 8. update the validity of inode with inode_idx and update_cnt.
	 *    this is the record of CLOSE event of COW
	 */
	((uint32_t *)tail_buf)[0] = pf->inode_idx;
	((uint32_t *)tail_buf)[1] = pf->update_cnt;
	if (io_ops.write(SLFS_INODE_TAB_START + pf->inode_idx * SLFS_INODE_SIZE,
			  SLFS_DATABLK_TAIL_SIZE, 
			  tail_buf))
		return -IO_ERR;

	return NO_ERR;
}

int slfs_read(slfs_file_t *pf, uint8_t *pbuf, uint32_t bytes_read)
{
	uint8_t tail_buf[SLFS_DATABLK_TAIL_SIZE] = {0, };
	uint32_t datablk_addr;
	uint32_t datablk_idx_pos;
	uint32_t buf_pos;
	uint32_t pos;
	uint32_t i;
	uint32_t offset;
	uint32_t addr;
	uint32_t len;

	if ((pf->file_size == 0) || (pf->datablk_addr == MINUS_ONE))
		return NO_ERR;

	if (pf->file_size < pf->pos + bytes_read)
		bytes_read = pf->file_size - pf->pos;

	pos = pf->pos;
	datablk_idx_pos = 0;
	while(pos > SLFS_DATABLK_PAYLOAD_SIZE) {
		datablk_idx_pos++;
		pos -= SLFS_DATABLK_PAYLOAD_SIZE;
		if (pos < SLFS_DATABLK_PAYLOAD_SIZE)
			 break;
	}
	datablk_addr = traverse_datablk_list(pf, datablk_idx_pos, NULL);
	/* read the head word */
	if (io_ops.read(datablk_addr + (pos & WD_MASK), sizeof(uint32_t), tail_buf))
		return -IO_ERR;

	offset = pf->pos & ~WD_MASK;
	for (i = 0; i < (sizeof(uint32_t) - offset); i++)
		pbuf[i] = tail_buf[i + offset];

	buf_pos = i;
	bytes_read -= i;
	pos = (pos & WD_MASK) + sizeof(uint32_t);
	/* reset the pos and recalc the datablk_addr */
	if (pos == SLFS_DATABLK_PAYLOAD_SIZE) {
		pos = 0;
		datablk_idx_pos++;
		datablk_addr = traverse_datablk_list(pf, datablk_idx_pos, NULL);
	}

	while (bytes_read >= sizeof(uint32_t)) {
		addr = datablk_addr + pos;
		if (bytes_read > (SLFS_DATABLK_PAYLOAD_SIZE - pos)) {
			len = SLFS_DATABLK_PAYLOAD_SIZE - pos;
			bytes_read -= (SLFS_DATABLK_PAYLOAD_SIZE - pos);
			datablk_addr = *((uint32_t *)(datablk_addr + SLFS_DATABLK_TAIL_LOW));
		} else if (bytes_read >= sizeof(uint32_t)) {
			len = (bytes_read & WD_MASK);
			bytes_read -= (bytes_read & WD_MASK);
		}
		buf_pos += len;
		if (io_ops.read(addr, len, &pbuf[buf_pos]))
			return -IO_ERR;
	}

	/* read the tail bytes
	 * fixme: update the addr with current read bytes first.
	 */
	if (0 != bytes_read) {
		/* end of the current datablk, then move to the next datablk */
		if ((addr & SLFS_DATABLK_MASK) == SLFS_DATABLK_PAYLOAD_SIZE)
			addr = *((uint32_t *)(datablk_addr + SLFS_DATABLK_TAIL_LOW));

		len = sizeof(uint32_t);
		if (io_ops.read(addr, len, tail_buf))
			return -IO_ERR;
		for (i = 0; i < bytes_read; i++)
			pbuf[buf_pos + i] = tail_buf[i];
	}

	return NO_ERR;
}

int slfs_delete(slfs_file_t *pf)
{
	int ret; 
	slfs_inode_t inode;

	ret = io_ops.read(SLFS_INODE_TAB_START + (pf->inode_idx * SLFS_INODE_SIZE), SLFS_INODE_SIZE, (uint8_t *)&inode);
	if (ret) 
		goto DELETE_FAIL;

	ret = update_fp(pf);
	if (ret)
		goto DELETE_FAIL;

	ret = collect_garbage();
	if (ret)
		goto DELETE_FAIL;

	ret = update_fp(pf);
	if (ret)
		goto DELETE_FAIL;

	/* write a zero-ed inode with the same inode name.
	 * the inode and datablks with the same inode name
	 * will be garbage collected afterwards.
	 */

	return NO_ERR;

DELETE_FAIL:
	return ret;
}

int slfs_get_next_file(slfs_file_t *pf, uint32_t *pinode_loc)
{
	uint32_t i;
	uint32_t j;
	uint32_t del_file_desc[SLFS_INODE_MAX_CNT];
	uint32_t file_id;
	bool bfound;
	slfs_inode_t inode;

	for (i = 0, j = 0; i < SLFS_INODE_MAX_CNT; i++) {
		if (io_ops.read(SLFS_INODE_TAB_START + (SLFS_INODE_SIZE * i), SLFS_INODE_SIZE, (uint8_t *)&inode))
			return -IO_ERR;
		/* record the deleted file's inode */
		if ((inode.file_size == 0) && (inode.datablk_addr == 0))
			del_file_desc[j++] = inode.file_id;
	}

	bfound = false;
	for (i = *pinode_loc; i < SLFS_INODE_MAX_CNT; i++) {
		if (io_ops.read(SLFS_INODE_TAB_START + (SLFS_INODE_SIZE * i), SLFS_INODE_SIZE, (uint8_t *)&inode))
			return -IO_ERR;
		if ((inode.inode_idx != MINUS_ONE) && (inode.update_cnt != MINUS_ONE)) {
			for (j = 0; j < SLFS_INODE_MAX_CNT; j++) {
				file_id = del_file_desc[j];
				/* found deleted file's inode */
				if (file_id != inode.file_id) {
					*pinode_loc = i;
					bfound = true;
					break;
				}
			}
		}
		if (bfound)
			break;
	}

	if (bfound) {
		pf->datablk_addr = inode.datablk_addr;
		pf->fd = inode.inode_idx;
		pf->file_size = inode.file_size;
		for (j = 0; j < SLFS_FNAME_LEN; j++) {
			pf->name[j] = inode.name[j];
			if ('\0' == inode.name[j])
				break;
		}
	}

	if (i == SLFS_INODE_MAX_CNT)
		return INODE_LAST_ERR;

	return NO_ERR;
}

int slfs_close(slfs_file_t *pf)
{
	if ((pf == NULL) || (pf->fd == MINUS_ONE))
		return NULL_PTR_ERR;

	pf->fd = MINUS_ONE;
	pf->datablk_addr = MINUS_ONE;
	pf->file_size = 0;
	pf->open_cnt--;
	pf->pos = 0;

	return NO_ERR;
}
