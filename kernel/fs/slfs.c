
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <ramdisk_io.h>
#include <slfs.h>

#define SLFS_MAGIC_STR_LEN		16
#define FNAME_LEN			32

/* inode_idx and update_cnt should be placed together
 * file_size and datablk_addr should be placed together 
 */
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
	/* absolute address of ramdisk */
	uint32_t superblk_start;
	/* from this all offset address */
	uint32_t inode_table_bmp_start;
	uint32_t datablk_bmp_start;
	uint32_t inode_table_start;
	uint32_t datablk_start;
	uint32_t storage_size;
	uint32_t free_size;
	uint32_t file_cnt;
	uint32_t blk_size;
	bool bmounted;
} slfs_superblk_t;

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
#define SLFS_INODE_MAX_CNT		(32U)
#define SLFS_INODE_SIZE			(sizeof(slfs_inode_t))
#define SLFS_INODE_TAB_START		(SLFS_DATABLK_BITMAP_START + SLFS_DATABLK_BITMAP_SIZE)
#define SLFS_INODE_TAB_SIZE		(SLFS_INODE_MAX_CNT * SLFS_INODE_SIZE)
/* garbage page table */
#define SLFS_GARBAGE_PAGE_TAB_START	(SLFS_INODE_TAB_START + SLFS_INODE_TAB_SIZE)
#define SLFS_GARBAGE_PAGE_TAB_MAX_CNT	(RAMDISK_PAGE_SIZE >> SLFS_PAGE_SIZE_SHIFT)
#define SLFS_GARBAGE_PAGE_TAB_SIZE	(SLFS_GARBAGE_PAGE_TAB_MAX_CNT << 2)
/* datablk */
#define SLFS_DATABLK_START		RAMDISK_PAGE_SIZE
#define SLFS_DATABLK_SIZE		RAMDISK_BLK_SIZE
#define SLFS_DATABLK_MASK		(SLFS_DATABLK_SIZE - 1)
#define SLFS_DATABLK_SIZE_SHIFT		RAMDISK_BLK_SIZE_SHIFT
#define SLFS_DATABLK_MAX_CNT		((RAMDISK_SIZE - SLFS_METADATA_SIZE) >> RAMDISK_BLK_SIZE_SHIFT)
#define SLFS_DATABLK_TAIL_SIZE		(8U)
#define SLFS_DATABLK_PAYLOAD_SIZE	(RAMDISK_BLK_SIZE - SLFS_DATABLK_TAIL_SIZE)
#define SLFS_DATABLK_TAIL_OFF_HI	(SLFS_DATABLK_PAYLOAD_SIZE)
#define SLFS_DATABLK_TAIL_OFF_LOW	(SLFS_DATABLK_PAYLOAD_SIZE + 4)

#define PAGEIDX(db)			(db >> 12)
#define PAGEADDR(db)			(db & 0xFFFFE000)

/* global buffer to store rw page data */
static uint8_t page_load_data[RAMDISK_PAGE_SIZE];
/* global buffer to store superblk */
static slfs_superblk_t superblk;
static const char magic_string[SLFS_MAGIC_STR_LEN] = "slfs filesystem";

/* COW (Copy On Write)
 * open event flag: file_size
 * close evet flag: update_cnt
 */

static int load_superblk(void)
{
	uint32_t i;
	uint8_t *pos = NULL;

	if (!io_ops.read(SLFS_SUPER_BLK_START, sizeof(slfs_superblk_t), (uint8_t*)&superblk))
		return -IO_ERR;

	pos = (uint8_t *)&(superblk.magic_string[0]);
	for (i = 0; i < SLFS_MAGIC_STR_LEN; i++) {
		if (pos[i] != magic_string[i])
			return -METADATA_ERR;
	}

	superblk.bmounted = true;

	return NO_ERR;
}

static int init_metadata(void)
{
	uint32_t i;
	uint32_t *pos;
	uint8_t *str;
	slfs_superblk_t superblk;

	str = (uint8_t *)(&superblk);
	for (i = 0; i < SLFS_MAGIC_STR_LEN; i++) {
		str[i] = magic_string[i];
	}

	/* Superblock start address */
	pos = (uint32_t *)(&str[i]);
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

	if (!io_ops.write(SLFS_METADATA_START, sizeof(slfs_superblk_t), (uint8_t *)&superblk))
		return -IO_ERR;

	return NO_ERR;
}

static int copy_flash_to_flash(uint32_t dst, uint32_t src, uint32_t len)
{
	uint8_t buf[SLFS_DATABLK_SIZE];
	uint32_t copy_len;
	uint32_t offset;

	/* 1. copy flash region to other flash region.
	 *    caller should erase the target flash region (page) first.
	 */

	offset = 0;
	while (len > 0) {
		if (len > SLFS_DATABLK_SIZE)
			copy_len = SLFS_DATABLK_SIZE;
		else
			copy_len = len;

		if (io_ops.read(src + offset, copy_len, buf))
			return -IO_ERR;

		if (io_ops.write(dst + offset, copy_len, buf))
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
	uint32_t offset;
	uint32_t garbage_page_idx;
	uint8_t garbage_page_section[SLFS_GARBAGE_PAGE_TAB_SIZE];
	int ret;

	if (SLFS_PAGE_CNT <= page_idx)
		return -PARAM_ERR;

	if (!bwrite) {
		addr = page_idx << SLFS_PAGE_SIZE_SHIFT;
		io_ops.read(addr, SLFS_PAGE_SIZE, page_load_data);
	} else {
		/* 1. erase swap page */
		if (io_ops.erase_page(SLFS_SWAP_PAGE_IDX))
			return -IO_ERR;

		/* 2. write target page backup to the swap page. 
		 *    if page_idx is the metadata page, the last
		 *    4 byte is reserved to store metadata page idx
		 *    to handle the backup swap page. This is the record
		 *    of target page and closure of writing to swap page 
		 *    of metadata page.
		 */
		if (page_idx == SLFS_METADATA_PAGE_IDX) { 
			offset = SLFS_PAGE_SIZE - sizeof(uint32_t);
			((uint32_t *)page_load_data)[offset >> 2] = (page_idx << SLFS_PAGE_SIZE_SHIFT);
		} 
		if (io_ops.write(SLFS_SWAP_PAGE_START, SLFS_PAGE_SIZE, page_load_data))
			return -IO_ERR;

		/* 3. if page_idx is normal data page, record it to metadata page garbage page section.
		 *    (if page_idx is metadata page, write its page address to the
		 *    last 4 byte of the temp page in step 2)
		 *    this is the closure of writing to temp page and
		 *    the temp page can be safely used to recover the 
		 *    original page during mount()
		 */
		if (page_idx != SLFS_METADATA_PAGE_IDX) {
			if (io_ops.read(SLFS_GARBAGE_PAGE_TAB_START, 
					SLFS_GARBAGE_PAGE_TAB_SIZE, 
					(uint8_t *)&garbage_page_section))
				return -IO_ERR;

			for (i = 0; i < SLFS_GARBAGE_PAGE_TAB_MAX_CNT; i++) {
				garbage_page_idx = ((uint32_t*)garbage_page_section)[i];
				if (garbage_page_idx == MINUS_ONE) {
					((uint32_t *)garbage_page_section)[i] = page_idx << SLFS_PAGE_SIZE_SHIFT;
					addr = SLFS_GARBAGE_PAGE_TAB_START + (i * sizeof(uint32_t));
					len = SLFS_GARBAGE_PAGE_TAB_SIZE;
					if (io_ops.write(addr, len, (uint8_t *)&garbage_page_section))
						return -IO_ERR;
					break;
				}
			}
		}
		
		/* 4. Erase the target page */
		if (io_ops.erase_page(page_idx))
			return -IO_ERR;

		/* 5. copy it from swap page to target page */
		ret = copy_flash_to_flash((page_idx << SLFS_PAGE_SIZE_SHIFT),
					  SLFS_SWAP_PAGE_START,
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

	/* 2. for all inode in the metadata, if there is 
	 *    a free file index, do allocate it to the new file descriptor.
	 *    file_size is the open record of CoW(Copy on Write).
	 *    If file_sizei in the inode is MINUS_ONE, this inode
	 *    is a pure free inode, isn't used before.
	 */
	for (i = 0; i < SLFS_INODE_MAX_CNT; i++) {
		if (pinode->file_size == MINUS_ONE) {
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
	uint8_t datablk_tail[SLFS_DATABLK_TAIL_SIZE];
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

	/* 1. calculate blks needed based on fp->pos and required len. */
	if ((pf->pos + len) > pf->file_size)
		pf->file_size = pf->pos + len;
	
	len = pf->file_size;
	blks_needed = 0U;

	while (SLFS_DATABLK_PAYLOAD_SIZE < len) {
		blks_needed++;
		if (SLFS_DATABLK_PAYLOAD_SIZE < len)
			len -= SLFS_DATABLK_PAYLOAD_SIZE;
		else
			break;
	}

	if (len != 0)
		blks_needed++;

	/* 2. for all datablks check, the tail 8bytes of current datablk, 
	 *    if it shows a free blk (0xFFFFFFFF 0xFFFFFFFF), update the
	 *    datablk linked list (prev, next) until the allocated
	 *    datablk reached blk required
	 */
	datablk_addr = SLFS_DATABLK_START;
	blks_alloced = 0;
	for (i = 0; i < SLFS_DATABLK_MAX_CNT; i++) {
		if (io_ops.read(datablk_addr + SLFS_DATABLK_TAIL_OFF_HI, SLFS_DATABLK_TAIL_SIZE, datablk_tail))
			return -IO_ERR;

		prev_datablk_addr = ((uint32_t *)datablk_tail)[0];
		next_datablk_addr = ((uint32_t *)datablk_tail)[1];

		if ((MINUS_ONE == prev_datablk_addr) && (MINUS_ONE == next_datablk_addr)) {
			/* 3. if free datablk found, write the file size and the first datablk addr 
			 *    into the inode. this ia an OPEN event of COW.
			 */
			if (blks_alloced == 0U) {
				/* 3-1. if this datablk is the first datablk alloced, update
				 *      the file_size and inode datablk_addr to point it
				 */
				((uint32_t *)datablk_tail)[0] = pf->file_size;
				((uint32_t *)datablk_tail)[1] = datablk_addr;

				pf->datablk_addr = datablk_addr;

				offset = OFFSET_OF(slfs_inode_t, file_size);
				/* pf->inode_idx should be set before allocating the datablks */
				addr = SLFS_INODE_TAB_START + (pf->inode_idx * SLFS_INODE_SIZE) + offset;
				/* update file_size and datablk_addr in inode, OPEN evt */
				if (io_ops.write(addr, SLFS_DATABLK_TAIL_SIZE, datablk_tail))
					return -IO_ERR;

				/* The very first datablk points the inode */
				prev_prev_datablk_addr = SLFS_INODE_TAB_START + (pf->inode_idx * SLFS_INODE_SIZE);
				prev_datablk_addr = datablk_addr;
			} else {
				/* 3-2. if this datablk is not the first datablk alloced
				 *      update prev, next datablk addr ptr 
				 */
				((uint32_t *)datablk_tail)[0] = prev_prev_datablk_addr;
				((uint32_t *)datablk_tail)[1] = datablk_addr;

				addr = prev_datablk_addr + SLFS_DATABLK_TAIL_OFF_HI;
				if (io_ops.write(addr, SLFS_DATABLK_TAIL_SIZE, datablk_tail))
					return -IO_ERR;

				prev_prev_datablk_addr = prev_datablk_addr;
				prev_datablk_addr = datablk_addr;
			}
			blks_alloced++;
		}

		if (blks_alloced == blks_needed) {
			pf->allocedblk_num = blks_alloced;
			/* 4. update the last datablk tail addresses. 
			 *    next address is 0.
			 */
			((uint32_t *)datablk_tail)[0] = prev_prev_datablk_addr;
			((uint32_t *)datablk_tail)[1] = 0;
			addr = datablk_addr + SLFS_DATABLK_TAIL_OFF_HI;
			if (io_ops.write(addr, SLFS_DATABLK_TAIL_SIZE, datablk_tail))
					return -IO_ERR;

			/* 5. datablk allocation done, exit loop */
			break;
		}

		datablk_addr += SLFS_DATABLK_SIZE;
	}

	if ((i == SLFS_DATABLK_MAX_CNT) && (blks_alloced != blks_needed))
		return -NO_DATABLK_ERR;

	return NO_ERR;
}

static int32_t get_datablk_num(slfs_file_t *pf, uint32_t *pdatablk_num)
{
	uint32_t cur_datablk_addr;
	uint32_t datablk_num;

	if (!pf || !pdatablk_num)
		return -NULL_PTR_ERR;

	datablk_num = 0;
	cur_datablk_addr = pf->datablk_addr;
	while (MINUS_ONE != cur_datablk_addr) {
		if (!io_ops.read(cur_datablk_addr + SLFS_DATABLK_TAIL_OFF_LOW,
				 sizeof(uint32_t),
				 (uint8_t *)&cur_datablk_addr))
			return -IO_ERR;

		datablk_num++;
	}

	*pdatablk_num = datablk_num;

	return NO_ERR;
}

static int32_t get_datablk_addr(slfs_file_t *pf,
			        uint32_t datablk_idx,
				uint32_t *pdatablk_addr)
{
	/* traverse the datablk list and 
	 * return the datablk address for datablk_idx
	 */
	uint32_t cur_datablk_addr;

	if (!pf)
		return -NULL_PTR_ERR;

	/* pf doesn't have any datablk alloced yet */
	if (MINUS_ONE == pf->datablk_addr)
		return -NO_DATABLK_ERR;

	cur_datablk_addr = pf->datablk_addr;
	while ((0U != datablk_idx) && (MINUS_ONE != cur_datablk_addr)) {
		if (!io_ops.read(cur_datablk_addr + SLFS_DATABLK_TAIL_OFF_LOW,
				 sizeof(uint32_t),
				 (uint8_t *)&cur_datablk_addr))
			return -IO_ERR;

		datablk_idx--;
	}

	if (cur_datablk_addr == MINUS_ONE) 
		return -NO_DATABLK_ERR;

	*pdatablk_addr = cur_datablk_addr;

	return NO_ERR;
}

static int delete_datablks(slfs_inode_t *pinode)
{
	uint32_t *pdatablk;
	uint32_t i;
	uint32_t datablk_addr;
	uint32_t cur_datablk_addr;
	uint32_t cur_page_idx;

	datablk_addr = pinode->datablk_addr;
	/* 1. go to the last datablk */
	while (MINUS_ONE != datablk_addr) {
		cur_datablk_addr = datablk_addr;
		datablk_addr = *((uint32_t *)(cur_datablk_addr + SLFS_DATABLK_TAIL_OFF_LOW));
		/* 1-1. compare prev addr of next datablk with current datablk address.
		 *      if they are not the same (broken linked list), 
		 *      next datablk is dangled, or was already deleted.
		 *      this is happening when powered off during delete_datablks().
		 */
		if (cur_datablk_addr != *((uint32_t *)(datablk_addr + SLFS_DATABLK_TAIL_OFF_HI)))
			return -LIST_BROKEN_ERR;
	}

	/* 2. traverse the datablk list back from the last datablk, erase it */
	do {
		cur_page_idx = PAGEIDX(cur_datablk_addr);
		/* 2-1. read(load) the page including current datablk */
		if (rw_page(cur_page_idx, false))
			return -IO_ERR;

		do {
			/* 2-2. erase the datablk in the memory-dumped page, 
			 *      offset address should be used 
			 */
			pdatablk = (uint32_t *)&page_load_data[cur_datablk_addr - PAGEADDR(cur_datablk_addr)];
			/* 2-3. read prev datablk addr */
			cur_datablk_addr = *((uint32_t *)(cur_datablk_addr + SLFS_DATABLK_TAIL_OFF_HI));
			for (i = 0; i < (SLFS_DATABLK_SIZE >> 4); i++) 
				pdatablk[i] = MINUS_ONE;

		/* 2-4. if prev datablk is still in current page, keep staying current loop */
		} while (cur_page_idx == PAGEIDX(cur_datablk_addr));

		/* 2-5. write back the updated page into storage */
		if (rw_page(cur_page_idx, true))
			return -IO_ERR;

	/* 2-6. The first datablk's prev points the inode and
	 *      its value should be less than the datablk start address.
	 */
	} while (SLFS_DATABLK_START <= cur_datablk_addr);

	return NO_ERR;
}

static int recover_page(uint32_t page_idx)
{
	uint32_t i;
	uint32_t *pdata;
	int ret;

	/* 1. recover_page always assumes the swap page has the right
	 *    data for recovering. erase target page first.
	 */
	if (io_ops.erase_page(page_idx))
		return -IO_ERR;

	/* 2. copy swap page into target page */
	ret = copy_flash_to_flash(page_idx,
				  SLFS_SWAP_PAGE_START,
				  SLFS_PAGE_SIZE);
	if (ret)
		return ret;

	/* 3. if data page idx, clean the garbage page section in the metadata page */
	if (page_idx != SLFS_METADATA_PAGE_IDX) {
		if (rw_page(SLFS_METADATA_PAGE_IDX, false))
			return -IO_ERR;
		for (i = 0; i < SLFS_GARBAGE_PAGE_TAB_MAX_CNT; i++) {
			pdata = (uint32_t *)&page_load_data[SLFS_GARBAGE_PAGE_TAB_START + (i * sizeof(uint32_t))];
			*pdata = MINUS_ONE;
		}
		if (rw_page(SLFS_METADATA_PAGE_IDX, true))
			return -IO_ERR;
	} 

	return NO_ERR;
}

static int slfs_memcpy(void *pdst, void *psrc, uint32_t len)
{
	uint32_t i;

	if ((NULL != psrc) && (NULL != pdst)) {
		for (i = 0; i < len; i++) 
			((uint8_t *)pdst)[i] = ((uint8_t *)psrc)[i];

		return NO_ERR;
	} else 
		return -NULL_PTR_ERR;
}

static int collect_garbage(void)
{
	uint32_t i;
	uint32_t j;
	uint32_t k;
	uint32_t del_file_desc[SLFS_INODE_MAX_CNT * 2];
	uint32_t deleted_fd_idx = 0;
	uint32_t file_id;
	uint32_t update_cnt;
	slfs_inode_t *pinode;
	slfs_inode_t inode;

	/* initialize del_file_desc */
	for (i = 0; i < SLFS_INODE_MAX_CNT * 2; i++) 
		del_file_desc[i] = MINUS_ONE;

	/* 1. collect obsolete inodes */
	for (i = 0; i < SLFS_INODE_MAX_CNT; i++) {
		if (io_ops.read(SLFS_INODE_TAB_START + i * SLFS_INODE_SIZE, SLFS_INODE_SIZE, (uint8_t *)&inode))
			return -IO_ERR;

		/* 1-1. file_size isn't updated, not OPEN evt for this inode */
		if (inode.file_size == MINUS_ONE)
			continue;

		/* 1-2. if current inode's update_cnt is MINUS_ONE, 
		 *      it is a dangled inode.
		 *      delete all datablks of current inode.
		 */
		if (inode.update_cnt == MINUS_ONE)
			delete_datablks(&inode);
		/* 1-3. record the deleted file's(file_size, datablk_addr are 0) inode */
		else if ((inode.file_size == 0) && (inode.datablk_addr == 0)) {
			del_file_desc[deleted_fd_idx++] = inode.file_id;
			del_file_desc[deleted_fd_idx++] = inode.update_cnt;
		}
	}

	deleted_fd_idx = 0;
	/* 2. delete datablks of deleted file */
	for (i = 0; i < SLFS_INODE_MAX_CNT; i++) {
		file_id = del_file_desc[deleted_fd_idx++];
		update_cnt = del_file_desc[deleted_fd_idx++];
		/* reach the end of deleted file desc */
		if (file_id == MINUS_ONE)
			break;

		for (j = 0; j < SLFS_INODE_MAX_CNT; j++) {
			if (!io_ops.read(SLFS_INODE_TAB_START + j * SLFS_INODE_SIZE, SLFS_INODE_SIZE, (uint8_t *)&inode))
				return -IO_ERR;

			/* should check file_id and update_cnt both */
			if ((inode.file_id == file_id) && (inode.update_cnt == update_cnt))
				delete_datablks(&inode);
		}
	}
	
	/* TODO: if powered-off here, there are incorrect garbage page
	 *       index in the metadata and it will overwrite the right
	 *       data with erased data in recovery() process.
	 */
	/* 3. update of the metadata page */
	if (rw_page(SLFS_METADATA_PAGE_IDX, false))
		return -IO_ERR;

	/* 3-1. clean the garbage page idx section */
	for (i = 0; i < (SLFS_GARBAGE_PAGE_TAB_SIZE >> 2); i++) 
		((uint32_t *)page_load_data)[(SLFS_GARBAGE_PAGE_TAB_START >> 2) + i] = MINUS_ONE;

	for (i = 0; i < SLFS_INODE_MAX_CNT; i++) {
		pinode = (slfs_inode_t *)(&page_load_data[SLFS_INODE_TAB_START + (i * SLFS_INODE_SIZE)]);
		/* 3-2. delete the dangled inode */
		if (pinode->update_cnt == MINUS_ONE) {
			for (j = 0; j < (SLFS_INODE_SIZE >> 2); j++)
				((uint32_t *)pinode)[j] = MINUS_ONE;
		}

		/* 3-3. delete the deleted files' inode */
		for (j = 0; j < (SLFS_INODE_MAX_CNT * 2); j+= 2) {
			file_id = del_file_desc[j];
			update_cnt = del_file_desc[j + 1];
			/* reach the end of deleted files' desc */
			if (file_id == MINUS_ONE)
				break;

			/* 3-4. found deleted file's inode and delete it */
			if ((pinode->file_id == file_id) && (pinode->update_cnt == update_cnt)) {
				for (k = 0; k < (SLFS_INODE_SIZE >> 2); k++)
					((uint32_t *)pinode)[k] = MINUS_ONE;

				break;
			}
		}
	}

	/* 4. update metadata page */
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
	uint32_t offset;
	int ret;

	/* 1. recover broken page if any */
	/* 1-1. check swap page tail */
	if (io_ops.read(SLFS_SWAP_PAGE_TAIL_START, SLFS_SWAP_PAGE_TAIL_SIZE, tail_buf))
		return -IO_ERR;

	/* 1-2. first, check the metadata page validity.
	 *      if swap page tail 4bytes are valid, metadata page update wasn't finished,
	 *      try to recover it.
	 */
	if (((uint32_t *)tail_buf)[0] == SLFS_METADATA_PAGE_IDX) {
		ret = recover_page(SLFS_METADATA_PAGE_IDX);
		if (ret)
			return ret;
	}
	/* 1-3. check if there is any pending page update from garbage page 
	 * 	table in the metadata. only the last entry is the right page 
	 * 	index that was stopped while being udpated. 
	 */
	else {
		/* load the metadata page into the global page_load_data buffer */
		if (rw_page(SLFS_METADATA_PAGE_IDX, false))
			return -IO_ERR;

		garbage_page_idx_prev = 0;
		for (i = 0; i < SLFS_GARBAGE_PAGE_TAB_MAX_CNT; i++) {
			offset = SLFS_GARBAGE_PAGE_TAB_START + (i * sizeof(uint32_t));
			garbage_page_idx = ((uint32_t *)page_load_data)[offset >> 2];
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

	/* 2. load superblk 
	 * 2-1. load magic string to check there a valid slfs.
	 */

	if (io_ops.read(SLFS_SUPERBLK_START, SLFS_MAGIC_STR_LEN, magic_str_buf))
		return -IO_ERR;

	for (i = 0; i < SLFS_MAGIC_STR_LEN; i++)
		if (magic_str_buf[i] != magic_string[i])
			break;

	/* Be cautious: if the flash storage doesn't have the right SLFS 
	 * signature string, it will be formatted. 
	 */
	if (i != SLFS_MAGIC_STR_LEN) {
		/* 2-2 format if it doesn't have a magic string */
		ret = slfs_format();
		if (ret)
			return ret;
		/* 2-3. init the metadata */
		ret = init_metadata();
		if (ret)
			return ret;
	}
	/* 2-4. load superblk */
	ret = load_superblk();
	if (ret)
		return ret;

	return NO_ERR;
}

int slfs_umount(void)
{
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
	uint32_t datablk_num;
	int32_t ret;
	slfs_inode_t inode;
	slfs_inode_t inode_last;

	/* 1. check if there is already a file descriptor in the inode table,
	 *    then record the first and latest inode having the same file name.
	 *    the update_cnt can be considered as a timestamp of the inode
	 */
	bfirst_found = true;
	inode_last.inode_idx = MINUS_ONE;
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

		if (bfound) {
			if (bfirst_found) {
				bfirst_found = false;
				ret = slfs_memcpy(&inode_last, &inode, sizeof(inode));
				if (ret)
					return ret;
			} else {
				/* if current inode is newer than inode_last, update inode_last with current */
				if (inode_last.update_cnt < inode.update_cnt) {
					ret = slfs_memcpy(&inode_last, &inode, sizeof(inode));
					if (ret)
						return ret;
				} 
			}
		}
	}

	/* 2. there is already a file created (the inode_last is valid),
	 *    copy the inode info to file descriptor
	 */
	if (inode_last.inode_idx != MINUS_ONE) {
		pf->fd = inode_last.file_id;
		pf->file_size = inode_last.file_size;
		pf->pos = 0U;
		pf->datablk_addr = inode_last.datablk_addr;
		pf->inode_idx = inode_last.inode_idx;
		pf->open_cnt++;
		for (i = 0; i < SLFS_FNAME_LEN; i++) {
			pf->name[i] = inode_last.name[i];
			if ('\0' == inode_last.name[i]) 
				break;
		}
		pf->update_cnt = inode_last.update_cnt;
		ret = get_datablk_num(pf, &datablk_num);
		if (ret)
			return ret;
	} else {
		/* 3. if there is not a file created, do a lazy inode allocation. 
		 *    if there is no inode entry, the inode is alloced only
		 *    when the file write runs because of the COW implementation.
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

	uint8_t data_buf[SLFS_DATABLK_SIZE];
	uint32_t i;
	uint32_t j;
	uint32_t datablk_addr;
	uint32_t buf_pos;
	uint32_t padded_bytes_write;
	uint32_t bytes_written;
	uint32_t pos;
	uint32_t datablk_addr_prev;
	uint32_t allocedblk_num_prev;
	uint32_t file_size_prev;
	uint32_t len;
	slfs_inode_t inode;
	int ret;

	if (!pf || (pbuf == NULL))
		return -NULL_PTR_ERR;

	if (pf->open_cnt == 0)
		return -FILE_PARAM_ERR;

	if (bytes_write == 0)
		return NO_ERR;

	/* 1-1. back up current pf info */
	datablk_addr_prev = pf->datablk_addr;
	file_size_prev = pf->file_size;
	allocedblk_num_prev = pf->allocedblk_num;
	/* 1-2. calculate the right file size with the file pos */
	if (pf->file_size == 0)
		pf->file_size = bytes_write;
	else if (pf->pos + bytes_write > pf->file_size)
		pf->file_size = pf->pos + bytes_write;

	/* 2. update the pf file descriptor with a new
	 *    free inode info in the inode table.
	 *    file write is creating a new inode from the previous inode.
	 */
	ret = update_fp(pf);
	/* 2-1. if there is not free inode in the inode table,
	 *      collect garbages and try again to find a free inode
	 */
	if (ret == -NO_INODE_ERR) {
		ret = collect_garbage();
		if (ret)
			return ret;
		/* 2-2. try again after garbage collected and 
		 *      if sill no free inode, then return error 
		 */
		ret = update_fp(pf);
		if (ret)
			return ret;
	} else if (ret)
		return ret;
	/* 3. allocate free datablks and connect them with doubly
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
	 *    write back the fp->name, inode_idx, file_id to the new alloc-ed inode
	 */
	inode.inode_idx = pf->inode_idx;
	inode.file_id = pf->fd;
	/* backup the file name */
	slfs_memcpy(inode.name, pf->name, SLFS_FNAME_LEN);
	if (io_ops.write((SLFS_INODE_TAB_START + 
			  pf->inode_idx * SLFS_INODE_SIZE),
			  SLFS_FNAME_LEN,
			  (uint8_t *)&inode))
		return -IO_ERR;

	/* 5. write the buffer data to the flash datablks for the first time */
	/* 5-1. this is determined only with the filesize.
	 *      there is no inode having file_size 0 in 
	 *      the inode table
	 */
	if (file_size_prev == 0) {
		/* 5-2. calc the padded byte data length */
		padded_bytes_write = bytes_write;
		if ((padded_bytes_write & ~DWD_MASK) != 0) {
			padded_bytes_write &= DWD_MASK;
			padded_bytes_write += SLFS_DATABLK_TAIL_SIZE;
		}
		datablk_addr = pf->datablk_addr;
		buf_pos = 0;
		while (padded_bytes_write > 0) {
			/* 5-3. if current data size is still bigger than 
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
						SLFS_DATABLK_TAIL_OFF_LOW);
			}
			/* 5-4. if current data size is less than one datablk size
			 *      write it to the datablk with right handling of tail data
			 */
			else {
				if (io_ops.write(datablk_addr,
					         padded_bytes_write,
						 (uint8_t *)(&pbuf[buf_pos])))
					return -IO_ERR;

				datablk_addr += padded_bytes_write;
				buf_pos += padded_bytes_write;
				padded_bytes_write = 0;
			}
		}
	}
	/* 6. if file_size is not 0, update (append, modify) the prev inode with file's fseek */
	else {
		datablk_addr = pf->datablk_addr;
		pos = pf->pos;
		bytes_written = 0;
		/* 6-1. copy all datablks from previous inode datablk.
		 *      while copying, update the new contents if it falls
		 *      into datablk currently being copied (update file content)
		 */
		buf_pos = 0;
		for (i = 0; i < allocedblk_num_prev; i++) {
			/* 6-1-1. copy prev file datablk content to data buf except datablk tail data */
			if (io_ops.read(datablk_addr_prev, SLFS_DATABLK_PAYLOAD_SIZE, data_buf))
				return -IO_ERR;

			/* 6-1.2. update the content if the pos or pos + len 
			 *        is in current datablk
			 */
			if ((bytes_write > 0) && (pos < SLFS_DATABLK_PAYLOAD_SIZE)) {
				len = SLFS_DATABLK_PAYLOAD_SIZE - pos;
				len = (len < bytes_write) ? len : bytes_write;
				for (j = 0; j < len; j++) 
					data_buf[pos + j] = pbuf[buf_pos + j];

				if (bytes_write > len)
					bytes_write -= len;
				else {
					bytes_write = 0;
					for (j = pos + j; j < SLFS_DATABLK_PAYLOAD_SIZE; j++)
						data_buf[j] = 0xFF;
				}
				/* reset pos to the start of datablk for the next writing */
				pos = 0;
				/* update the write buf_pos with written len */
				buf_pos += len;
			} else 
				pos -= SLFS_DATABLK_PAYLOAD_SIZE;

			/* 6-1-3. write only payload data from data_buf back to
			 *        the new file desc's datablks.
			 *        the prev, next address in the new datablks were
			 *        already updated in the alloc_datablk().
			 */
			if (io_ops.write(datablk_addr, SLFS_DATABLK_PAYLOAD_SIZE, data_buf))
				return -IO_ERR;

			bytes_written += len;
			/* 6-1-4. move to the next datablk in each prev 
			 *        file and current file
			 */
			datablk_addr_prev = *((uint32_t *)(datablk_addr_prev + SLFS_DATABLK_TAIL_OFF_LOW));
			datablk_addr = *((uint32_t *)(datablk_addr + SLFS_DATABLK_TAIL_OFF_LOW));
		}
		/* 6-2. if there are still bytes to be written (file append) */
		if (bytes_write > 0) {
			for (i = 0; i < (pf->allocedblk_num - allocedblk_num_prev); i++) {
				if (bytes_write < SLFS_DATABLK_PAYLOAD_SIZE) {
					len = bytes_write;
					bytes_write = 0;
				} else {
					len = SLFS_DATABLK_PAYLOAD_SIZE;
					bytes_write -= SLFS_DATABLK_PAYLOAD_SIZE;
				}

				if (io_ops.write(datablk_addr, len, &pbuf[bytes_written + (i * SLFS_DATABLK_PAYLOAD_SIZE)]))
					return -IO_ERR;

				if (bytes_write == 0)
					break;

				datablk_addr = *((uint32_t *)(datablk_addr + SLFS_DATABLK_TAIL_OFF_LOW));
				bytes_written += len;
			}
		}
	}
	
	/* 7. update the validity of inode with update_cnt.
	 *    this is the record of CLOSE event of COW
	 */
	if (io_ops.write(SLFS_INODE_TAB_START + 
			 pf->inode_idx * SLFS_INODE_SIZE +
			 OFFSET_OF(slfs_inode_t, update_cnt),
			 sizeof(uint32_t), 
			 (uint8_t *)&(pf->update_cnt)))
		return -IO_ERR;

	return NO_ERR;
}

int slfs_read(slfs_file_t *pf, uint8_t *pbuf, uint32_t bytes_read)
{
	uint32_t datablk_addr;
	uint32_t datablk_idx_pos;
	uint32_t buf_pos;
	uint32_t offset;
	uint32_t len;
	uint32_t ret;

	if (!pbuf)
		return -NULL_PTR_ERR;

	if ((pf->file_size == 0) || (pf->datablk_addr == MINUS_ONE))
		return -FILE_SIZE_ERR;

	if (pf->file_size < pf->pos + bytes_read)
		bytes_read = pf->file_size - pf->pos;

	datablk_idx_pos = 0;
	/* 1-1. find datablk index including pos */
	while(offset > SLFS_DATABLK_PAYLOAD_SIZE) {
		datablk_idx_pos++;
		offset -= SLFS_DATABLK_PAYLOAD_SIZE;
		if (offset < SLFS_DATABLK_PAYLOAD_SIZE)
			 break;
	}
	/* 1-2. get datablk address for pos */
	ret = get_datablk_addr(pf, datablk_idx_pos, &datablk_addr);
	if (ret)
		return ret;

	/* 2. read data from current datablk */
	len = SLFS_DATABLK_PAYLOAD_SIZE - offset;
	if (len > bytes_read)
		len = bytes_read;

	buf_pos = 0;
	if (io_ops.read(datablk_addr + offset, len, &pbuf[buf_pos]))
		return -IO_ERR;

	buf_pos += len;
	bytes_read -= len;

	/* 3. read remaing data from next datablks */
	while (bytes_read > 0) {
		if (io_ops.read(datablk_addr + SLFS_DATABLK_TAIL_OFF_LOW, 
			        sizeof(uint32_t), 
				(uint8_t *)&datablk_addr))
			return -IO_ERR;
		len = bytes_read;
		if (len >= SLFS_DATABLK_PAYLOAD_SIZE)
			len = SLFS_DATABLK_PAYLOAD_SIZE;
		if (io_ops.read(datablk_addr, len, &pbuf[buf_pos]))
			return -IO_ERR;

		bytes_read -= len;
		buf_pos += len;
	}

	return NO_ERR;
}

int slfs_delete(slfs_file_t *pf)
{
	int ret; 
	slfs_inode_t inode;

	/* 1. load current inode_idx to inode */
	if (io_ops.read(SLFS_INODE_TAB_START + (pf->inode_idx * SLFS_INODE_SIZE), 
			SLFS_INODE_SIZE, 
			(uint8_t *)&inode))
		return -IO_ERR;

	/* 2. update pf with new inode from inode table */
	ret = update_fp(pf);
	if (ret == -NO_INODE_ERR) {
		/* 2-1. if there is not free inode anymore, collect garbage */
		ret = collect_garbage();
		if (ret)
			return ret;

		/* 2-2. update pf again */
		ret = update_fp(pf);
		if (ret)
			return ret;
	} else if (ret)
		return ret;

	/* 3. write a zero-ed inode with the same inode name.
	 *    the inode and datablks with the same inode name
	 *    will be garbage collected afterwards.
	 */
	inode.inode_idx = pf->inode_idx;
	inode.update_cnt = pf->update_cnt;
	inode.file_size = 0;
	inode.datablk_addr = 0;

	if (io_ops.write(SLFS_INODE_TAB_START + pf->inode_idx * SLFS_INODE_SIZE,
			 SLFS_INODE_SIZE,
			 (uint8_t *)&inode))
		return -IO_ERR;

	return NO_ERR;
}

int slfs_get_next_file(slfs_file_t *pf, uint32_t *pinode_loc)
{
	uint32_t i;
	uint32_t j;
	uint32_t del_file_desc[SLFS_INODE_MAX_CNT];
	uint32_t file_id;
	bool bfound;
	slfs_inode_t inode;

	/* 1. gather the deleted file's file_id */
	for (i = 0, j = 0; i < SLFS_INODE_MAX_CNT; i++) {
		if (io_ops.read(SLFS_INODE_TAB_START + (SLFS_INODE_SIZE * i), SLFS_INODE_SIZE, (uint8_t *)&inode))
			return -IO_ERR;
		/* record the deleted file's file_id from inode */
		if ((inode.file_size == 0) && (inode.datablk_addr == 0))
			del_file_desc[j++] = inode.file_id;
	}

	bfound = false;
	/* 2. for inode from current input inode, find next valid inode 
	 *    which isn't equal to the deleted inode file_id 
	 */
	for (i = *pinode_loc; i < SLFS_INODE_MAX_CNT; i++) {
		if (io_ops.read(SLFS_INODE_TAB_START + (SLFS_INODE_SIZE * i), SLFS_INODE_SIZE, (uint8_t *)&inode))
			return -IO_ERR;

		if ((inode.inode_idx != MINUS_ONE) && (inode.update_cnt != MINUS_ONE)) {
			for (j = 0; j < SLFS_INODE_MAX_CNT; j++) {
				file_id = del_file_desc[j];
				/* found 's inode that isn't from deleted file */
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

	/* 3. update file pointer with next valid inode info */
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
