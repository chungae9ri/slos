// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

/**
 * @addtogroup kernel
 * @{
 * @addtogroup kernel_core Core
 * @{
 * @addtogroup kernel_core_mm Memory management
 * @{
 *
 * @file
 *
 * @brief Page table initialization, page fault handle and free pages
 *
 */

#include <page_table.h>
#include <frame_pool.h>
#include <mem_layout.h>

/** Pointer to current page table */
static struct pagetable *pcurrentpgt;

void init_pageregion(struct pagetable *ppagetable, struct framepool *pframepool,
		     const unsigned int _shared_size)
{
	ppagetable->paging_enabled = 0;
	ppagetable->pframepool = pframepool;
	ppagetable->shared_size = _shared_size;
	ppagetable->ppage_dir = 0;
	ppagetable->ppage_table = 0;
	ppagetable->VMcnt = 0;
}

/* small page translation is used */
void init_pagetable(struct pagetable *ppagetable, PG_TYPE pagetype)
{
	if (pagetype == PG_TABLE_KERN && ppagetable->ppage_dir)
		return;

	/* Page directory is located in process mem pool
	 * 16KB for 4096 entry(16KB = 4K * 4Byte) is needed
	 */
	if (pagetype == PG_TABLE_USER) {
		/* Do nothing for now */
	} else {
		/* Assign prealloced 4 contiguous memory frames
		 * for page_directory : 4K entry
		 */
		ppagetable->ppage_dir = (unsigned int *)KERN_PGD_START_BASE;
		ppagetable->ppage_table = (unsigned int *)KERN_PGT_START_BASE;
	}
}

void load_pagetable(struct pagetable *ppagetable)
{
	unsigned int r0 = 0;

	/* Invalidate tlb */
	asm("mcr p15, 0, %0, c8, c7, 0" : : "r"(r0) :);
	pcurrentpgt = ppagetable;
}

void handle_fault(void)
{
	int frameno;
	unsigned int pgdIdx, pgtIdx;
	unsigned int *pda;
	unsigned int *pfa;
	unsigned int *pde, *pte, *frame_addr;

	/* Read DFAR */
	asm("mrc p15, 0, %0, c6, c0, 0" : "=r"(pfa)::);
	/* Read ttb0 */
	asm("mrc p15, 0, %0, c2, c0, 0" : "=r"(pda)::);

	/* Entry for 1st level descriptor */
	pgdIdx = ((unsigned int)pfa & 0xFFF00000) >> 20;
	pde = (unsigned int *)(((unsigned int)pda) + (pgdIdx << 2));

	/* Section fault */
	if ((*pde & 0x00000003) == 0x0) {
		*pde = (KERN_PGT_START_BASE + ((pgdIdx << 8) << 2)) | 0x1E1;
	}

	/* Kernel heap fault only */
	frameno = get_frame(pcurrentpgt->pframepool);
	frame_addr = (unsigned int *)FRAMETOPHYADDR(frameno);

	/* Entry for 2nd level descriptor */
	pgtIdx = ((unsigned int)pfa & 0x000FF000) >> 12;
	pte = (unsigned int *)(KERN_PGT_START_BASE + ((pgdIdx << 8) << 2) + (pgtIdx << 2));
	/* Set the value of 2nd level descriptor */
	*pte = ((unsigned int)frame_addr | 0x472);
}

void free_page(unsigned int freedAddr)
{
	unsigned int r0 = 0;
	unsigned int pgdIdx, pgtIdx;
	unsigned int pda, *pte;
	unsigned int *frame_addr;
	unsigned int frame_num;
	/* Read ttb */
	asm("mrc p15, 0, %0, c2, c0, 0" : "=r"(pda)::);
	/* Entry for 1st level descriptor */
	pgdIdx = ((unsigned int)freedAddr & 0xFFF00000) >> 20;
	/*pde = (unsigned int *)(((unsigned int)pda) + (pgdIdx << 2));*/

	/* Entry for 2nd level descriptor */
	pgtIdx = ((unsigned int)freedAddr & 0x000FF000) >> 12;
	pte = (unsigned int *)(KERN_PGT_START_BASE + ((pgdIdx << 8) << 2) + (pgtIdx << 2));

	/* Physical address of frame */
	frame_addr = (unsigned int *)(*pte);
	frame_num = (unsigned int)(frame_addr) >> 12;

	if (frame_num >= HEAP_FRAME_START && frame_num < HEAP_FRAME_START + HEAP_FRAME_NUM) {
		release_frame(pcurrentpgt->pframepool, frame_num);
	} else {
		/* error !
		 * only frames in the heap can be freed.
		 */
	}

	*pte = 0x0;
	/* Should not clean the 1st level entry
	 * Invalidate tlb
	 */
	asm("mcr p15, 0, %0, c8, c7, 0" : : "r"(r0) :);
}

/**
 * @}
 * @}
 * @}
 *
 */
