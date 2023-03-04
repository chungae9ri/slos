#include <slfs.h>

#define SLFS_SUPER_BLK_START	RAMDISK_START
#define SLFS_SUPER_BLK_SIZE	RAMDISK_SECT_SIZE

#define SLFS_MAGIC_STR_LEN	16
#define FNAME_LEN		32

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

static slfs_filesystem_t slfs_inst;
static uint8_t superblk[SLFS_SUPER_BLK_SIZE];
static const char magic_string[SLFS_MAGIC_STR_LEN] = "slfs filesystem";

static bool load_superblock(void)
{
	uint32_t i = 0U;
	uint32_t *pos = NULL;

	pRxDesc->addr = SLFS_SUPER_BLK_START;
	pRxDesc->len = SLFS_SUPER_BLK_SIZE;
	pRxDesc->pdata = superblk;
	Ret = read_ramdisk_blk();

	for (i = 0; i < RAMDISK_SECT_SIZE / RAMDISK_BLK_SIZE; i++)
		if (!read_ramdisk_blk(i, &superblk[i * RAMDISK_BLK_SIZE]))
			return false;

	for (i = 0; i < SLFS_MAGIC_STR_LEN; i++) {
		if (superblk[i] != magic_str[i])
			return false;

		slfs_inst.magic_string[i] = superblk[i];
	}

	pPos = (uint32_t *)&superblk[MAGIC_STR_LEN];
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
	return true;
}

static bool update_fp(slfs_file_t *pf)
{
	/* 1. load the metadata page */

	/* 2. for all inode in the metadata, if it is 
	 *    a free file index, do allocate it to the 
	 *    new file descriptor 
	 *    FileSize and DataBlkAddr is the record of CoW(Copy on Write)
	 *    If one of these isn't MINUS_ONE, that means 
	 *    the file with that iNode should be one of
	 *    dangled file, oudated file, deleted file or
	 *    a valid file. 
	 */
	return true;
}

static bool alloc_datablk(slfs_file_t *pf, uint32_t BytesWrite)
{
	/* 1. calculate blk needed based on fp->pos and BytesWrite.
	 * 2. for all datablks check the tail 8bytes of current datablk, 
	 *    if it shows a free blk (0xFFFFFFFF 0xFFFFFFFF), update the
	 *    datablk linked list (prev, next) until the allocated
	 *    datablk reached blk required
	 */
	return true;
}

static uint32_t traverse_datablk_list(slfs_file_t *pf,
		uint32_t DataBlkIdx,
		uint32_t *pNewDataBlkIdx)
{
	/* 1. traverse the list of current file pf.
	 *    return the last datablk address
	 */

	return true;
}

static bool copy_flash_to_flash(uint32_t SrcAddr, uint32_t DstAddr, uint32_t Len)
{
	/* 1. copy flash blk to flash bok */

	return true;
}

static bool delete_datablks(slfs_inode_t *piNode)
{
	/* 1. go to the last datablk  
	 * 2. traverse the datablk list from the last datablk,
	 *    erase it 
	 */
	return true;
}

static bool rw_page(uint32_t PageIdx, bool bWrite)
{
	/* 1. erase the temp page 
	 * 2. write back current page to the temp page 
	 * 3. record target page Idx.
	 *    if PageIdx is a metadata page, write it to the 
	 *    last 8 byte of the temp page.
	 *    if PageIdx is one of data pages, write it to the
	 *    metadata garbage page section.
	 *    this is the closure of writing to temp page and
	 *    the temp page can be safely used to recover the 
	 *    original page in mount().
	 *
	 * 4. erase the target page 
	 * 5. copy it form temp page to target page 
	 * 6. erase the temp page 
	 */
	return true;
}

static bool recover_page(uint32_t PageIdx)
{

	/* 1. copy temp page into target page
	 * 2. if metadata page idx, just clean up the temp page.
	 *    if data page idx, clean the garbage page section
	 *    in the metadata page and clean the temp page
	 */
	return true;
}

static bool io_read(uint32_t Addr, uint32_t pData, uint32_t Len)
{
	return true;
}

static bool io_write(uint32_t Addr, uint32_t pData, uint32_t Len)
{
	return true;
}

static bool memcpy(void *pDst, void *pSrc, uint32_t Len)
{
	return true;
}


bool slfs_mount(uint8_t *pPageLoadData)
{
	/* 1. check the swap page if there is a valid data in the swap page
	 * 2. if metadata page update wasn't finished, recover metadata page
	 * 3. check if there is any pending page update from 
	 *    garbage page table in the metadata.
	 *    only the last entry is the right page index that was stopped 
	 *    while being updated.
	 * 4. Load the superblk
	 * 5. format if it doesn't have a magic string
	 * 6. init meta data
	 * 7. load super blk 
	 */

	return true;

MOUNT_FAIL:
	return false;
}

bool slfs_umount(void)
{
	return true;
}

bool slfs_format(void)
{
	/* File system format does erase all contents in 
	 * the metadata page (superblock, inode table, etc)
	 * and all datablk pages
	 */
	return true;
}

bool slfs_open(const uint8_t *pName, slfs_file_t *pf)
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
	return true;
}

bool slfs_seek(slfs_file_t *pf, 
		uint32_t Offset,
		slfs_fseek_t Whence)
{
	return true;
}

bool slfs_write(slfs_file_t *pf,
		uint8_t *pBuf,
		uint32_t BytesWrite)
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
