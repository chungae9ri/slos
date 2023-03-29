/* COW (Copy On Write)
 * open event flag: file size, datablk_addr
 * close evet flag: inode_idx, update_cnt
 */
#include <ramdisk_io.h>
#include <slfs.h>

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

static uint8_t page_load_data[RAMDISK_PAGE_SIZE];
static slfs_filesystem_t slfs_inst;
static uint8_t superblk[SLFS_DATABLK_SIZE];
static const char magic_string[SLFS_MAGIC_STR_LEN] = "slfs filesystem";

#define SLFS_SUPER_BLK_START	RAMDISK_START

#define SLFS_MAGIC_STR_LEN	16
#define FNAME_LEN		32
#define MINUS_ONE		(0xFFFFFFFF)
#define OFFSET_OF(st, m)	(uint32_t)(&(((st *)0)->m))

#define SLFS_PAGE_SIZE_SHIFT		RAMDISK_PAGE_SIZE_SHIFT
#define SLFS_STORAGE_SIZE		RAMDISK_SIZE
#define SLFS_PAGE_SIZE			RAMDISK_PAGE_SIZE
/* swap page */
#define SLFS_SWAP_PAGE_IDX		(RAMDISK_PAGE_NUM - 1)
#define SLFS_SWAP_PAGE_START		(SLFS_SWAP_PAGE_IDX << SLFS_PAGE_SIZE_SHIFT)
#define SLFS_SWAP_PAGE_SIZE		RAMDISK_PAGE_SIZE
#define SLFS_SWAP_PAGE_TAIL_SIZE	(4)
#define SLFS_SWAP_PAGE_TAIL_START	(SLFS_SWAP_PAGE_START + SLFS_SWAP_PAGE_SIZE - SLFS_SWAP_PAGE_TAIL_SIZE)
/* metadata page */
#define SLFS_METADATA_PAGE_IDX		0
#define METADATA_SIZE			RAMDISK_PAGE_SIZE
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
#define SLFS_INODE_TAB_START		(SLFS_DATABLK_BITMAP_START + SLFS_DATABLK_BITMAP_SIZE)
#define SLFS_INODE_TAB_SIZE		(SLFS_INODE_MAX_CNT * SLFS_INODE_SIZE)
/* garbage page table */
#define SLFS_GARBAGE_PAGE_TAB_START	(SLFS_INODE_TAB_START + SLFS_INODE_TAB_SIZE)
#define SLFS_GARBAGE_PAGE_TAB_SIZE	(RAMDISK_PAGE_SIZE - SLFS_INODE_TAB_START)
#define SLFS_GARBAGE_PAGE_MAX_CNT	(SLFS_GARBAGE_PAGE_TAB_SIZE >> 3)
/* datablk */
#define SLFS_DATABLK_START		RAMDISK_PAGE_SIZE
#define SLFS_DATABLK_SIZE		RAMDISK_BLK_SIZE
#define SLFS_DATABLK_SIZE_SHIFT		RAMDISK_BLK_SIZE_SHIFT
#define SLFS_DATABLK_MAX_CNT		((RAMDISK_SIZE - METADATA_SIZE) >> RAMDISK_BLK_SIZE_SHIFT)
#define SLFS_DATABLK_TAIL_SIZE		8
#define SLFS_DATABLK_PAYLOAD_SIZE	(RAMDISK_BLK_SIZE - SLFS_DATABLK_TAIL_SIZE)
#define SLFS_DATABLK_TAIL_HI		(SLFS_DATABLK_PAYLOAD_SIZE)
#define SLFS_DATABLK_TAIL_LOW		(SLFS_DATABLK_PAYLOAD_SIZE + 4)
#define SLFS_DATABLK_MAX_CNT		((RAMDISK_SIZE - RAMDISK_PAGE_SIZE) >> 9)

/**/
#define PAGEIDX(db)			(db >> 12)
#define PAGEADDR(db)			(db & 0xFFFFE000)
/* ramdisk layout */

static bool load_superblk(void)
{
	uint32_t i;
	uint32_t *pos = NULL;

	if (!read_ramdisk_blk(0, superblk))
		return false;

	for (i = 0; i < SLFS_MAGIC_STR_LEN; i++) {
		if (superblk[i] != magic_str[i])
			return false;

		slfs_inst.magic_string[i] = superblk[i];
	}

	pPos = (uint32_t *)&superblk[SLFS_MAGIC_STR_LEN];
	slfs_inst.superblk_start = *pPos++;
	slfs_inst.inode_table_bmp_start = *pPos++;
	slfs_inst.datablk_bmp_start = *pPos++;
	slfs_inst.inode_table_start = *pPos++;
	slfs_inst.datablk_start = *pPos++;
	slfs_inst.storage_size = *pPos++;
	slfs_inst.free_size = *pPos++;
	slfs_inst.file_cnt = *pPos++;
	slfs_inst.blk_size = RAMDISK_BLK_SIZE;
	slfs_inst.bmounted = true;

	return true;
}

static bool init_metadata(void)
{
	uint8_t *pos;
	/* let's reuse superblk */
	pos = superblk;
	for (i = 0; i < SLFS_MAGIC_STR_LEN; i++) {
		pos[i] = magic_string[i];
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
	*pos++ = SLFS_DATA_START;
	/* Data storage total size in bytes */
	*pos++ = SLFS_DATABLK_PAYLOAD_SIZE * SLFS_DATABLK_MAX_CNT;
	/* Data storage free size in bytes */
	*pos++ = SLFS_DATABLK_PAYLOAD_SIZE * SLFS_DATABLK_MAX_CNT;
	/* File count */
	*pos = 0U;

	for (i = 0; i < RAMDISK_SECT_SIZE / RAMDISK_BLK_SIZE; i++)
		if (!write_ramdisk_blk(i, &superblk[i * RAMDISK_BLK_SIZE]))
			return false;
	return true;
}

static bool update_fp(slfs_file_t *pf)
{
	uint32_t i;
	slfs_inode_t *pinode;

	/* 1. load the metadata page */
	if (!rw_page(SLFS_METADATA_PAGE_IDX, false))
		return false;

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
		return false;

	return true;
}

static bool alloc_datablk(slfs_file_t *pf, uint32_t len)
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
		return false;

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
		if (!read_ramdisk(datablk_addr + SLFS_DATABLK_TAIL_HI, sizeof(uint32_t), &prev_datablk_addr))
			goto DATABLK_ALLOC_FAIL;

		if (!read_ramdisk(datablk_addr + SLFS_DATABLK_TAIL_LOW, sizeof(uint32_t), &next_datablk_addr))
			goto DATABLK_ALLOC_FAIL;

		if ((MINUS_ONE == prev_datablk_addr) && (MINUS_ONE == next_datablk_addr)) {
			/* writing the file size and the first datablk addr into the inode
			 * this ia an OPEN event of COW.
			 */
			if (blks_alloced == 0U) {
				pf->datablk_addr = datablk_addr;
				offset = OFFSET_OF(slfs_inode_t, file_size);
				addr = SLFS_INODE_TAB_START + (pf->inode_idx * SLFS_INODE_SIZE) + offset;
				if (!write_ramdisk(addr, sizeof(uint32_t), &pf->file_size))
					goto DATABLK_ALLOC_FAIL;

				offset = OFFSET_OF(slfs_inode_t, datablk_addr);
				addr = SLFS_INODE_TAB_START + (pf->inode_idx * SLFS_INODE_SIZE) + offset;
				if (!write_ramdisk(addr, sizeof(uint32_t), &datablk_addr))
					goto DATABLK_ALLOC_FAIL;

				prev_prev_datablk_addr = SLFS_INODE_TAB_START + (pf->inode_idx * sizeof(struct slfs_inode_t));
				prev_datablk_addr = datablk_addr;
			} else {
				/* update prev datablk addr ptr */
				addr = prev_datablk_addr + SLFS_DATABLK_TAIL_HI;
				if (!write_ramdisk(addr, sizeof(uint32_t), &prev_prev_datablk_addr))
					goto DATABLK_ALLOC_FAIL;

				addr = prev_datablk_addr + SLFS_DATABLK_TAIL_LOW;
				if (!write_ramdisk(addr, sizeof(uint32_t), &datablk_addr))
					goto DATABLK_ALLOC_FAIL;

				prev_prev_datablk_addr = prev_datablk_addr;
				prev_datablk_addr = datablk_addr;
			}
			blks_alloced++;
		}
		if (blks_alloced == blks_needed) {
			pf->alloced_blk_num = blks_alloced;
			/* update the last datablk addr ptrs */
			addr = datablk_addr + SLFS_DATABLK_TAIL_HI;
			if (!write_ramdisk(addr, sizeof(uint32_t), &prev_prev_datablk_addr))
					goto DATABLK_ALLOC_FAIL;

			addr = datablk_addr + SLFS_DATABLK_TAIL_LOW;
			prev_datablk_addr = 0;
			if (!write_ramdisk(addr, sizeof(uint32_t), &prev_datablk_addr))
					goto DATABLK_ALLOC_FAIL;
		}
		datablk_addr += SLFS_DATABLK_SIZE;
	}

	if ((i == SLFS_DATABLK_MAX_CNT) && (blks_alloced != blks_needed))
		goto DATABLK_ALLOC_FAIL;

	return true;

DATABLK_ALLOC_FAIL:
	return false;
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
		if (!read_ramdisk(cur_datablk_addr + SLFS_DATABLK_TAIL_LOW, 
					sizeof(uint32_t), 
					&next_datablk_addr)) 
			return MINUS_ONE;

		while ((0U != datablk_idx) && (MINUS_ONE != next_datablk_addr)) {
			cur_datablk_addr = next_datablk_addr;
			if (!read_ramdisk(cur_datablk_addr + SLFS_DATABLK_TAIL_LOW,
						sizeof(uint32_t),
						&next_datablk_addr))
				return MINUS_ONE;

			datablk_idx--;

			if (pnew_datablk_idx)
				(*pnew_datablk_idx)++;
		}
	}

	return cur_datablk_addr;
}

static bool copy_flash_to_flash(uint32_t src_addr, uint32_t dst_addr, uint32_t len)
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

		if (!read_ramdisk(src_addr + offset, copy_len, buf))
			return false;

		if (!write_ramdisk(dst_addr + offset, copy_len, buf))
			return false;

		len -= copy_len;
		offset += copy_len;
	}

	return true;
}

static bool delete_datablks(slfs_inode_t *pinode)
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
		if (!rw_page(cur_page_idx, false))
			return false;

		do {
			pdatablk = (uint32_t *)&page_load_data[cur_datablk_addr - PAGEADDR(cur_datablk_addr)];
			cur_datablk_addr = *((uint32_t *)(cur_datablk_addr + SLFS_DATABLK_TAIL_HI));
			for (i = 0; i < (SLFS_DATABLK_SIZE >> 4); i++) 
				pdatablk[i] = MINUS_ONE;

			if (cur_datablk_addr == 0U)
				break;
		} while (cur_page_idx == PAGEIDX(cur_datablk_addr));

		rw_page(cur_page_idx, true);
	} while (SLFS_DATA_START <= cur_datablk_addr);

	return true;
}

static bool rw_page(uint32_t page_idx, bool bwrite)
{
	uint32_t i;
	uint32_t addr;
	uint32_t len;
	uint32_t garbage_page_idx;
	uint8_t buff[SLFS_DATABLK_TAIL_SIZE];

	if (FEE_PAGE_CNT <= page_idx)
		goto RW_PAGE_ERROR;

	if (!bwrite)  {
		addr = page_idx << SLFS_PAGE_SIZE_SHIFT;
		read_ramdisk(addr, SLFS_PAGE_SIZE, page_load_data);

	}
	else {
		/* 1. erase temp page */
		if (!erase_ramdisk_page(SLFS_SWAP_PAGE_IDX))
			goto RW_PAGE_ERROR;

		/* 2. write back to the temp page */
		if (page_idx == 0) { /* if page_idx is metadata page */
			addr = SLFS_SWAP_PAGE_START;
			len = SLFS_PAGE_SIZE - SLFS_DATABLK_TAIL_SIZE;
		}
		else {
			addr = SLFS_SWAP_PAGE_START;
			len = SLFS_PAGE_SIZE;
		}
		write_ramdisk(addr, len, page_load_data);

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
			buff[0] = page_idx << SLFS_PAGE_SIZE_SHIFT;
			buff[4] = MINUS_ONE;
			if (!write_ramdisk(addr, len, buff)) 
				goto RW_PAGE_ERROR;
		} else {
			for (i = 0; i < SLFS_GARBAGE_PAGE_MAX_CNT; i++) {
				garbage_page_idx = *((uint32_t*)(SLFS_GARBAGE_PAGE_TAB_START + (i * sizeof(uint64_t))));
				if (garbage_page_idx == MINUS_ONE) {
					buff[0] = page_idx << SLFS_PAGE_SIZE_SHIFT;
					buff[4] = MINUS_ONE;
					addr = SLFS_GARBAGE_PAGE_TAB_START + (i * sizeof(uint64_t));
					len = SLFS_DATABLK_TAIL_SIZE;
					if (!write_ramdisk(addr, len, buff))
						goto RW_PAGE_ERROR;
					break;
				}
			}
		}
		/* 4. Erase the target page */
		if (!erase_ramdisk_page(page_idx))
			goto RW_PAGE_ERROR;
		/* 5. copy it from swap page to target page */
		if (!copy_flash_to_flash(SLFS_SWAP_PAGE_START, 
					 (page_idx << SLFS_PAGE_SIZE_SHIFT),
					 SLFS_PAGE_SIZE))
			goto RW_PAGE_ERROR;
		/* 6. erase swap page */
		if (!erase_ramdisk_page(SLFS_SWAP_PAGE_IDX))
			goto RW_PAGE_ERROR;

	}
	return true;

RW_PAGE_ERROR:
	return false;
}

static bool recover_page(uint32_t page_idx)
{
	/* 1. copy swap page into target page
	 * 2. if metadata page idx, just clean up the swap page.
	 *    if data page idx, clean the garbage page section
	 *    in the metadata page and clean the swap page
	 */
	uint32_t i;
	uint32_t *pdata;

	if (!erase_ramdisk_page(SLFS_METADATA_PAGE_IDX))
		goto RECOVER_PAGE_ERROR;

	if (!copy_flash_to_flash(SLFS_SWAP_PAGE_START, 
				 SLFS_METADATA_START, 
				 SLFS_PAGE_SIZE))
		goto RECOVER_PAGE_ERROR;

	if (page_idx != SLFS_METADATA_PAGE_IDX) {
		if (!rw_page(SLFS_METADATA_PAGE_IDX, false))
			goto RECOVER_PAGE_ERROR;
		for (i = 0; i < SLFS_GARBAGE_PAGE_TAB_MAX_CNT; i++) {
			pdata = (uint32_t *)&page_load_data[SLFS_GARBAGE_PAGE_TAB_START + (i * sizeof(uint64_t))];
			*pdata = MINUS_ONE;
			pdata++;
			*pdata = MINUS_ONE;
		}
		if (!rw_page(SLFS_METADATA_PAGE_IDX, true))
			goto RECOVER_PAGE_ERROR;
	} else {
		if(!erase_ramdisk_page(SLFS_SWAP_PAGE_IDX))
			goto RECOVER_PAGE_ERROR;
	}

	return true;

RECOVER_PAGE_ERROR:
	return false;
}

static bool slfs_memcpy(void *pdst, void *psrc, uint32_t len)
{
	uint32_t i = 0;

	if ((NULL != psrc) && (NULL != pdst)) {
		for (i = 0; i < len; i++) 
			((uint8_t *)pdst)[i] = ((uint8_t *)psrc)[i];

		return true;
	} else 
		return false;
}


bool slfs_mount(void)
{
	uint8_t magic_str_buf[SLFS_MAGIC_STR_LEN];
	uint8_t buff[SLFS_DATABLK_TAIL_SIZE] = {0, };
	uint32_t i;
	uint32_t addr;
	uint32_t len;
	uint32_t garbage_page_idx;
	uint32_t garbage_page_idx_prev;

	/* 1. check the swap page if there is a valid data in the swap page */
	if (!read_ramdisk(SLFS_SWAP_PAGE_TAIL_START, SLFS_SWAP_PAGE_TAIL_SIZE, buff))
		goto MOUNT_FAIL;

	/* 1-1. metadata page update wasn't finished */
	if (((uint32_t *)buff)[0] == SLFS_METADATA_PAGE_IDX) {
		if (!recover_page(SLFS_METADATA_PAGE_IDX))
			goto MOUNT_FAIL;
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
			if  (!recover_page(garbage_page_idx_prev))
				goto MOUNT_FAIL;
		}
	}
	/* 2. load the superblk */
	if (!read_ramdisk(SLFS_SUPERBLK_START, SLFS_MAGIC_STR_LEN, magic_str_buf))
		goto MOUNT_FAIL;

	for (i = 0; i < SLFS_MAGIC_STR_LEN; i++)
		if (magic_str_buf[i] != magic_string[i])
			break;
	/* Be cautious: if the flash storage doesn't have the right SLFS signature string, it will be formatted. */
	if (i != SLFS_MAGIC_STR_LEN) {
		/* 2-1. format if it doesn't have a magic string */
		if (!slfs_format())
			goto MOUNT_FAIL;
		/* 2-2. init the metadata */
		if (!init_metadata())
			goto MOUNT_FAIL;
	}
	/* 2-3. load superblk */
	if (!load_superblk())
		goto MOUNT_FAIL;

	return true;

MOUNT_FAIL:
	return false;
}

bool slfs_umount(void)
{
	slfs_inst.bmounted = false;

	return true;
}

bool slfs_format(void)
{
	/* File system format does erase all contents in 
	 * the metadata page (superblock, inode table, etc)
	 * and all datablk pages
	 */
	return erase_ramdisk_chip();
}

bool slfs_open(const uint8_t *pname, slfs_file_t *pf)
{
	/* 1. check if there is already a file descriptor 
	 *    in the iNode table, then record the first and 
	 *    latest iNode having the same file name.
	 *    use timestamp (updatecnt) of the iNode.
	 * 2. if here is already a file created
	 *    do Lazy iNode allocation.
	 *    If there is no iNode entry, the iNode is alloced only 
	 *    when the fwrite runs because of the COW implementation
	 */
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
		if (!read_ramdisk(SLFS_INODE_TAB_START + i * sizeof(inode), sizeof(inode), &inode))
			goto OPEN_FAIL;

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
	if (inodelast.inode_idx != MINUS_ONE) {
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
		if (pf->alloced_blk_num == MINUS_ONE)
			pf->alloced_blk_num = 0;
		else
			pf->alloced_blk_num++;
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
		pf->alloced_blk_num = 0;
		pf->update_cnt = 0;
		for (i = 0; i < SLFS_FNAME_LEN; i++) {
			pf->name[i] = pname[i];
			if ('\0' == pname[i])
				break;
		}
	}

	return true;

OPEN_FAIL:
	return false;
}

bool slfs_seek(slfs_file_t *pf, uint32_t offset, slfs_fseek_t whence)
{
	uint32_t pos;

	if (!pf)
		return false;
	if (!pf->open_cnt)
		return false;

	switch (whence) {
	case SLFS_SEEK_SET:
		pos = offset;
		if (pos > pf->file_size)
			return false;
		else 
			pf->pos = pos;
		break;
	case SLFS_SEEK_CUR:
		pos = pf->pos + offset;
		if (pos > pf->file_size)
			return false;
		else 
			pf->pos = pos;
		break;
	case SLFS_SEEK_END:
		pos = pf->file_size - offset;
		if (pos > pf->file_size)
			return false;
		else
			pf->pos = pos;
		break;
	default:
		break;
	}

	return true;
}

bool slfs_write(slfs_file_t *pf, uint8_t *pbuf, uint32_t bytes_write)
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
	return true;
}

bool slfs_read(slfs_file_t *pf,
		uint8_t *pBuf,
		uint32_t BytesRead)
{
	return true;
}

bool slfs_delete(slfs_file_t *pf)
{
	return true;
}

bool slfs_collect_garbage(void)
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

	return true;
}

bool slfs_get_next_file(slfs_file_t *pf, uint32_t *piNodeLoc)
{
	return true;
}

bool slfs_close(slfs_file_t *pf)
{
	return true;
}
