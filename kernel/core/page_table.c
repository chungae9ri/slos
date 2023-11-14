/*
  kernel/core/page_table.c page table manager
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

#include <page_table.h>
#include <frame_pool.h>
#include <mem_layout.h>

#define ENABLE_MMU		0x00000001
#define ENABLE_DCACHE		0x00000004
#define ENABLE_ICACHE		0x00001000
#define MASK_MMU		0x00000001
#define MASK_DCACHE		0x00000004
#define MASK_ICACHE		0x00001000

#define MMIO_START_ADDR		0xF8000000 /* size : 128MB */

static struct pagetable *pcurrentpgt;

void init_pageregion(struct pagetable *ppagetable,
		     struct framepool *pframepool,
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

	/* page directory is located in process mem pool 
	 * 16KB for 4096 entry(16KB = 4K * 4Byte) is needed
	 */
	if (pagetype == PG_TABLE_USER) {
		/* do nothing for now */
	} else {
		/* 
		 * assign prealloced 4 contiguous memory frames 
		 * for page_directory : 4K entry 
		 */
		ppagetable->ppage_dir = (unsigned int *)KERN_PGD_START_BASE_NS;
		ppagetable->ppage_table = (unsigned int *)KERN_PGT_START_BASE_NS;
	}
}

void load_pagetable(struct pagetable *ppagetable)
{
	unsigned int r0 = 0;

	/* invalidate tlb */
	asm ("mcr p15, 0, %0, c8, c7, 0" : :"r" (r0) :);
	pcurrentpgt = ppagetable;
}

void handle_fault(void)
{
	int frameno;
	unsigned int pgdIdx, pgtIdx;
	unsigned int *pda;
	unsigned int *pfa;
	unsigned int *pde, *pte, *frame_addr;
	/*unsigned int *page_table;*/
	/*unsigned int *pcur;*/

	/* read DFAR */
	asm ("mrc p15, 0, %0, c6, c0, 0" : "=r" (pfa) ::);
	/* read ttb0 */
	asm ("mrc p15, 0, %0, c2, c0, 0" : "=r" (pda) ::);
#if 0
	/* check fault address is in valid VM region */
	for (i = 0; i < VMcnt; i++) {
		if(pVMref[i]->is_legitimate((unsigned int)pfa)) break;
	}

	/* the fault address is not in valid VM region */
	if(VMcnt != 0 && i == VMcnt) {
		return;
	}
#endif
	/* entry for 1st level descriptor */
	pgdIdx = ((unsigned int)pfa & 0xFFF00000) >> 20;
	pde = (unsigned int *)(((unsigned int)pda) + (pgdIdx << 2));

	/* section fault */
	if ((*pde & 0x00000003) == 0x0) {
		*pde = (KERN_PGT_START_BASE_NS + ((pgdIdx << 8 ) << 2)) | 0x1E1;
	}

	/* kernel heap fault only */
	frameno = get_frame(pcurrentpgt->pframepool);
	frame_addr = (unsigned int *)FRAMETOPHYADDR(frameno);

	/* entry for 2nd level descriptor */
	pgtIdx = ((unsigned int)pfa & 0x000FF000) >> 12;
	pte = (unsigned int *)(KERN_PGT_START_BASE_NS + ((pgdIdx << 8) << 2) + (pgtIdx << 2));
	/* set the value of 2nd level descriptor */
	*pte = ((unsigned int)frame_addr | 0x472);
}

#if 0
void PageTable::register_vmpool(VMPool *_pool)
{
	pVMref[VMcnt++] = _pool;
}
#endif

void free_page(unsigned int freedAddr)
{
	unsigned int r0 = 0;
	unsigned int pgdIdx, pgtIdx;
	unsigned int pda, *pte;
	unsigned int *frame_addr;
	unsigned int frame_num;
	/* read ttb */
	asm ("mrc p15, 0, %0, c2, c0, 0" : "=r" (pda) ::);
	/* entry for 1st level descriptor */
	pgdIdx = ((unsigned int)freedAddr & 0xFFF00000) >> 20;
	/*pde = (unsigned int *)(((unsigned int)pda) + (pgdIdx << 2));*/

	/* entry for 2nd level descriptor */
	pgtIdx = ((unsigned int)freedAddr & 0x000FF000) >> 12;
	pte = (unsigned int *)(KERN_PGT_START_BASE_NS + ((pgdIdx << 8) << 2) + (pgtIdx << 2));

	/* physical address of frame */
	frame_addr = (unsigned int *)(*pte);
	frame_num = (unsigned int)(frame_addr) >> 12;

	if (frame_num >= HEAP_FRAME_START &&
 	   frame_num < HEAP_FRAME_START + HEAP_FRAME_NUM) {
		release_frame(pcurrentpgt->pframepool, frame_num);
	} else {
		/* error !
		 * only frames in the heap can be freed.
		 */
	}

	*pte = 0x0;
	/*
	 * should not clean the 1st level entry
	 */
	/**pde = 0x0;*/

	/* invalidate tlb */
	asm ("mcr p15, 0, %0, c8, c7, 0" : :"r" (r0) :);
}
