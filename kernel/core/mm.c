/*
  kernel/core/mm.c memory manager
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

#include <stdlib.h>
#include <stdint.h>
#include <frame_pool.h>
#include <page_table.h>
#include <mem_layout.h>
#include <mm.h>

extern int __kernel_heap_start__;
extern int __kernel_heap_end__;

struct vmpool *pvm_kernel;
struct vmpool *pvm_user;

void init_pgt(void)
{
	unsigned int i, j;
	unsigned int *pcur, *ppage_dir, *ppage_tbl;

	/* 
	 * assign prealloced 4 contiguous memory frames 
	 * for page_directory : 4K entry 
	 */
	ppage_dir = (unsigned int *)KERN_PGD_START_BASE;

	/* 
	 * each entry of page directory has 1MB memory addressing.
	 * 4G VM address range = 4K Entries * 1MB.
	 * There are 4 4KB frames for page directory.
	 * initialize page directory entry as 0. 
	 */
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 1024; j++) {
			ppage_dir[i * 1024 + j] = 0;
		}
	}

	/* 
	 * 6MB directly mapped memory for kernel
	 * 0 ~ 1MB: system reserved
	 * 1 ~ 2MB: kernel text, data, 
	 * 2 ~ 215000KB: stacks
	 * 215000KB ~ 6MB: bitmap, PGD, PGTs
	 * 6 ~ 8MB: kernel heap
	 * 8 ~ 9MB: usr task1 text, data, stack
	 * 9 ~ 12MB: usr task1 PGD, PGTs
	 * ...
	 * ? ~ ?MB: ramdisk img
	 */			
	pcur = (unsigned int *)KERN_PGT_START_BASE;
	for (i = 0; i < 4; i++) { 
		for (j = 0; j < 1024; j++) {
			/* 
			 * 0x1E1 is
			 * Bit[1:0] = 2'b01 : 00:fualt, 01:page, 10:section, 11 : reserved
			 * Bit[2] = 1'b0 : PXN (Privilege eXectuion Never)
			 * Bit[3] = 1'b0 : NS (Non-Secure)
			 * Bit[4] = 1'b0 : SBZ (Shoulb Be Zero)
			 * Bit[8:5] = 4'b1111 : Domain 0xF
			 * Bit[9] = 0 : don't care
			 */
			/* 
			 * 4K * 4 : offset between the start addr of PGD and PGT
			 */
			ppage_dir[i * 1024 + j] = 
				((unsigned int)pcur + (i * 1024 + j) * 1024) | 0x1E1;
		}
	}

	/* remap pgd for kernel heap area 64MB (0xC4000000 ~ 0xC8000000).
	 * entries for kernel heap is not allocated in page table.
	 * lazy allocator. set zero.
	 */
	for (i = 0; i < 64; i++) {
		ppage_dir[1024 * 3 + 64 + i] = 0x0;
	}

	/* 
	 * assign prealloced 4 contiguous memory frames 
	 * for page_directory : 4K entry 
	 */
	ppage_tbl = (unsigned int *)KERN_PGT_START_BASE;

	/* 
	 * each entry of page directory has 1MB memory addressing.
	 * 4G VM address range = 4K Entries * 1MB.
	 * There are 4 4KB frames for page directory.
	 * initialize page directory entry as 0. 
	 */
	for (i = 0; i < 1024 * 4; i++) {
		for (j = 0; j < 256; j++) {
			ppage_tbl[i * 256 + j] = 0;
		}
	}

	/* 
	 * for kernel with direct mapped address.
	 * 0x00000000 ~ 0xBFFFFFFF: 3GB, directly mapped address.
	 * 0xC0000000 ~ 0xDFFFFFFF: 512MB, Kernel virtual address.
	 * 0xE0000000 ~ 0xFFFFFFFF: 512MB, System(M_AXI_GP0/1, I/O peri, SLCR, 
	 *                          PS system reg, cpu private reg) directly mapped address.
	 */
	/* The 4GB direct mapped address */
	for (i = 0; i < 1024 * 4; i++) {
	/*for (i = 0; i < 0xF89; i++) {*/
		for (j = 0; j < 256; j++) {
			/* 0x472 is
	 		* Bit[0] = 1'b0 : XN(eXecution Never)
	 		* Bit[1] = 1'b1 : 0: Large page, 1: Small page
	 		* Bit[2] = 1'b0 : Bufferable
	 		* Bit[3] = 1'b0 : Cacheable
	 		* Bit[5:4] = 2'b11: AP[1:0] R/W full access with AP[2]=1'b0
	 		* Bit[8:6] = 3'b001: TEX[2:0] should be 001 with C = 1'b0, B = 1'b0. 
	                      		This is Outer and Inner Non Cacheable mode
	 		* Bit[9] = 1'b0: AP[2] should be 0 for full access
	 		* Bit[10] = 1'b1: S: shareable
	 		* Bit[11] = 1'b0: nG(non-Global) bit. 0 for global
	 		*/
			ppage_tbl[i * 256 + j] = ((i * 256 + j) * 4096) | 0x472;
		}
	}

	/* 
	 * remap kernel area(0xC0000000 ~ 0xDFFFFFFF) to 0x00000000 ~ 0x20000000 physical address
	 */
	for (i = (0xC00 * 256), j = 0; i < (0xC00 * 256) + 0x1FFFF; i++, j++) {
		ppage_tbl[i] = (j * 4096) | 0x472;
	}

	/* 
	 * remap cpu private register.
	 * cpu private register(0xF8900000~0xF8F02FFFF) must be Device or Strongly-ordered area
	 * in Cortex-A9 MPCore TRM
	 */
	for (i = (0xF89 * 256), j = 0; i < (0xF89 * 256) + 0x602; i++, j++) {
			/* 0x432 is
	 		* Bit[0] = 1'b0 : XN(eXecution Never)
	 		* Bit[1] = 1'b1 : 0: Large page, 1: Small page
	 		* Bit[2] = 1'b0 : Bufferable, 0 for Device or Strongly-ordered memory
	 		* Bit[3] = 1'b0 : Cacheable, 0 for Device or Strongly-ordered memory
	 		* Bit[5:4] = 2'b11: AP[1:0] R/W full access with AP[2]=1'b0
	 		* Bit[8:6] = 3'b000: TEX[2:0] should be 000 for Device or Strongly-ordered memory
	 		* Bit[9] = 1'b0: AP[2] should be 0 for full access
	 		* Bit[10] = 1'b1: S: shareable
	 		* Bit[11] = 1'b0: nG(non-Global) bit. 0 for global
	 		*/
			ppage_tbl[i] = (0xF8900000 + (j * 4096)) | 0x432;
	}

	/* remap page table for kernel heap area 64MB (0xC4000000 ~ 0xC8000000).
	 */
	for (i = 0; i < 64 * 256; i++) {
		ppage_tbl[0xC4000 + i] = 0;
	}

	return;
}

void init_kernmem(struct framepool *kfp, 
		struct pagetable *pgt, 
		struct vmpool *kheap)
{
	/* initialize kernel frame pools */
	init_framepool(kfp, KERN_FRAME_START, 
			KERN_FRAME_NUM, 0); 

#if 0
	mark_prealloc_frame(kfp, 
			PREALLOC_FRAME_START, 
			PREALLOC_FRAME_NUM);
#endif

	init_pageregion(pgt, kfp, 0 MB);
	init_pagetable(pgt, PG_TABLE_KERN);
	load_pagetable(pgt);

	/* 
	 * Since mmu is on, should use virtual address.
	 * Heap memory starts from address 64MB,size 64MB.
	 */
	init_vmpool(kheap, pgt, 
			(unsigned int)(&__kernel_heap_start__), 
			(unsigned int)(&__kernel_heap_end__) - (unsigned int)(&__kernel_heap_start__));
	/*init_vmpool(&pheap, &pgt, 1 GB, 112 MB);*/

	pvm_kernel = kheap;
	/*pvm_user = &pheap;*/
}

void *kmalloc(uint32_t size)
{
#if 0
	static uint8_t *heap = NULL;
	uint8_t *prev_heap;

	if (!heap) {
		heap = (uint8_t *)(&__kernel_heap_start__);
	}

	prev_heap = heap;
	if ((int)(heap + size) >= (int)(&__kernel_heap_end__)) {
		return 0;
	}
	heap += size;
	return (void *) prev_heap;
#else
	return (void *)(allocate(pvm_kernel, size));
#endif
}

void kfree(uint32_t addr)
{
	release(pvm_kernel, addr);
}

void *_sbrk(uint32_t size)
{
#if 0
	static uint8_t *heap = NULL;
	uint8_t *prev_heap;

	if (!heap) {
		heap = (uint8_t *)(&__kernel_heap_start__);
	}

	prev_heap = heap;
	if ((int)(heap + size) >= (int)(&__kernel_heap_end__)) {
		return 0;
	}
	heap += size;
	return (void *) prev_heap;
#else
	return (void *)(allocate(pvm_kernel, size));
#endif
}
