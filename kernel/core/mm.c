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
 * @file mm.c
 *
 * @brief Kernel memory management module
 *
 */

#include <stdint.h>

#include <frame_pool.h>
#include <page_table.h>
#include <mem_layout.h>
#include <mm.h>

struct vmpool *pvm_kernel;
struct vmpool *pvm_user;

/**
 * @brief Initialize page table
 *
 */
void init_pgt(void)
{
	unsigned int i, j;
	unsigned int *pcur, *ppage_dir, *ppage_tbl;

	/* Assign prealloced 4 contiguous memory frames
	 * for page_directory : 4K entry
	 */
	ppage_dir = (unsigned int *)KERN_PGD_START_BASE;

	/* Each entry of page directory has 1MB memory addressing.
	 * 4G VM address range = 4K Entries * 1MB.
	 * There are 4 4KB frames for page directory.
	 * initialize all page directory entries with 0.
	 */
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 1024; j++) {
			ppage_dir[i * 1024 + j] = 0;
		}
	}

	/* Set pgd entries with corresponding pgt base address.
	 * 16MB directly mapped memory for kernel
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
			/* 0x1E1 is
			 * Bit[1:0] = 2'b01 : 00:fualt, 01:page, 10:section, 11 : reserved
			 * Bit[2] = 1'b0 : PXN (Privilege eXectuion Never)
			 * Bit[3] = 1'b0 : NS (Non-Secure)
			 * Bit[4] = 1'b0 : SBZ (Shoulb Be Zero)
			 * Bit[8:5] = 4'b1111 : Domain 0xF
			 * Bit[9] = 0 : don't care
			 */
			/* 4K * 4 : offset between the start addr of PGD and PGT */
			ppage_dir[i * 1024 + j] =
			    ((unsigned int)pcur + (i * 1024 + j) * 1024) | 0x1E1;
		}
	}

	/* Remap pgd for kernel heap area 64MB (0xC4000000 ~ 0xC8000000).
	 * Entries for kernel heap is not allocated in page table.
	 * Lazy allocator. set zero.
	 */
	for (i = 0; i < 64; i++) {
		ppage_dir[1024 * 3 + 64 + i] = 0x0;
	}

	/* Assign prealloced 4 contiguous memory frames
	 * for page_directory : 4K entry
	 */
	ppage_tbl = (unsigned int *)KERN_PGT_START_BASE;

	/* Each entry of page directory has 1MB memory addressing.
	 * 4G VM address range = 4K Entries * 1MB.
	 * There are 4 4KB frames for page directory.
	 * Initialize all page table entries with 0.
	 */
	for (i = 0; i < 1024 * 4; i++) {
		for (j = 0; j < 256; j++) {
			ppage_tbl[i * 256 + j] = 0;
		}
	}

	/* For kernel with direct mapped address.
	 * 0x00000000 ~ 0x3FFFFFFF: 1GB, external RAM memory, directly mapped address except blanked
				    heap 0x04000000 ~ 0x08000000(64MiB) area.
	 * 0x40000000 ~ 0xBFFFFFFF: 2GB M_AXI_GP0/1 bufferable, not cacheable, directly mapped
	 * 0xC0000000 ~ 0xDFFFFFFF: 512MB, Kernel virtual address, remapped to 0x00000000 ~
	 0x20000000.
	 * 0xE0000000 ~ 0xE02FFFFF: 3MB memory mapped peripheral IO registers
	 * 0xE1000000 ~ 0xFFFFFFFF: Directly mapped to SMC, SLCR, PS system reg, cpu private reg,
	 *                          not bufferable, not cacheable
	 */
	/* The 1GB RAM direct mapped address */
	for (i = 0; i < 1024; i++) {
		for (j = 0; j < 256; j++) {
			/* 0x47E is
			 * Bit[0] = 1'b0 : XN(eXecution Never)
			 * Bit[1] = 1'b1 : 0: Large page, 1: Small page
			 * Bit[2] = 1'b1 : Bufferable
			 * Bit[3] = 1'b1 : Cacheable
			 * Bit[5:4] = 2'b11: SCTLR.AFE is 0. AP[1:0] R/W full access with AP[2]=1'b0
			 * Bit[8:6] = 3'b001: TEX[2:0] should be 001 with C = 1'b0, B = 1'b0.
			 This is Outer and Inner Non Cacheable mode
			 * Bit[9] = 1'b0: AP[2] should be 0 for full access
			 * Bit[10] = 1'b1: S: shareable
			 * Bit[11] = 1'b0: nG(non-Global) bit. 0 for global
			 */
			ppage_tbl[i * 256 + j] = ((i * 256 + j) * 4096) | 0x47E;
		}
	}

	/* M_AXI_GP0/1 mapped address */
	for (i = 0; i < 1024 * 3; i++) {
		for (j = 0; j < 256; j++) {
			/* M_AXI_GP0/1 (bufferable, not cacheable)
			 * 0x476 is
			 * Bit[0] = 1'b0 : XN(eXecution Never)
			 * Bit[1] = 1'b1 : 0: Large page, 1: Small page
			 * Bit[2] = 1'b1 : Bufferable
			 * Bit[3] = 1'b0 : Cacheable
			 * Bit[5:4] = 2'b11: SCTLR.AFE is 0. AP[1:0] R/W full access with AP[2]=1'b0
			 * Bit[8:6] = 3'b001: TEX[2:0] should be 001 with C = 1'b0, B = 1'b0.
			 This is Outer and Inner Non Cacheable mode
			 * Bit[9] = 1'b0: AP[2] should be 0 for full access
			 * Bit[10] = 1'b1: S: shareable
			 * Bit[11] = 1'b0: nG(non-Global) bit. 0 for global
			 */
			ppage_tbl[i * 256 + j] = ((i * 256 + j) * 4096) | 0x476;
		}
	}

	/* Kernel virtual address
	 * Remap kernel area(0xC0000000 ~ 0xDFFFFFFF) to 0x00000000 ~ 0x1FFFFFFF physical address
	 */
	for (i = (0xC00 * 256), j = 0; i < (0xC00 * 256) + 0x20000; i++, j++) {
		/* Bufferable, cacheable
		 * 0x47E is
		 * Bit[0] = 1'b0 : XN(eXecution Never)
		 * Bit[1] = 1'b1 : 0: Large page, 1: Small page
		 * Bit[2] = 1'b1 : Bufferable
		 * Bit[3] = 1'b1 : Cacheable
		 * Bit[5:4] = 2'b11: SCTLR.AFE is 0. AP[1:0] R/W full access with AP[2]=1'b0
		 * Bit[8:6] = 3'b001: TEX[2:0] should be 001 with C = 1'b0, B = 1'b0.
		 This is Outer and Inner Non Cacheable mode
		 * Bit[9] = 1'b0: AP[2] should be 0 for full access
		 * Bit[10] = 1'b1: S: shareable
		 * Bit[11] = 1'b0: nG(non-Global) bit. 0 for global
		 */
		ppage_tbl[i] = (j * 4096) | 0x47E;
	}

	/* Remap IOP address.
	 * IO peripheral registers (0xE0000000 ~ 0xE02FFFFF, 3MiB) is set for
	 * device IO memory bufferable, not cacheable
	 */
	for (i = (0xE00 * 256), j = 0; i < (0xE00 * 256) + 0x300; i++, j++) {
		/* 0x433 is
		 * Bit[0] = 1'b0 : XN(eXecution Never)
		 * Bit[1] = 1'b1 : 0: Large page, 1: Small page
		 * Bit[2] = 1'b1 : Bufferable, 0 for Device or Strongly-ordered memory
		 * Bit[3] = 1'b0 : Cacheable, 0 for Device or Strongly-ordered memory
		 * Bit[5:4] = 2'b11: AP[1:0] R/W full access with AP[2]=1'b0
		 * Bit[8:6] = 3'b000: TEX[2:0] should be 000 for Device or Strongly-ordered memory
		 * Bit[9] = 1'b0: AP[2] should be 0 for full access
		 * Bit[10] = 1'b1: S: shareable
		 * Bit[11] = 1'b0: nG(non-Global) bit. 0 for global
		 */
		ppage_tbl[i] = (0xE0000000 + (j * 4096)) | 0x436;
	}

	/* 0xE1000000 ~ 0xFFFFFFFF: Directly mapped to SMC, SLCR, PS system reg,
	 * cpu private reg, not bufferable, not cacheable cpu private
	 * register(0xE1000000~0xFFFFFFFF) must be Device or Strongly-ordered
	 * area in Cortex-A9 MPCore TRM
	 */
	for (i = (0xE10 * 256), j = 0; i < (0xE10 * 256) + 0x1F000; i++, j++) {
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
		ppage_tbl[i] = (0xE1000000 + (j * 4096)) | 0x432;
	}

	/* Remap page table for kernel heap area 64MB (0xC4000000 ~ 0xC8000000). */
	for (i = 0; i < 64 * 256; i++) {
		ppage_tbl[0xC4000 + i] = 0;
	}
}

void init_kernmem(struct framepool *kfp, struct pagetable *pgt, struct vmpool *kheap)
{
	/* initialize kernel frame pools */
	init_framepool(kfp, KERN_FRAME_START, KERN_FRAME_NUM, 0);

	init_pageregion(pgt, kfp, 0);
	init_pagetable(pgt, PG_TABLE_KERN);
	load_pagetable(pgt);

	/* Since mmu is on, should use virtual address.
	 * Heap memory starts from address 64MB,size 64MB.
	 */
	init_vmpool(kheap, pgt, (unsigned int)(&__kernel_heap_start__),
		    (unsigned int)(&__kernel_heap_end__) - (unsigned int)(&__kernel_heap_start__));

	pvm_kernel = kheap;
}

void *kmalloc(size_t size)
{
	return (void *)(allocate(pvm_kernel, size));
}

void kfree(uint32_t addr)
{
	release(pvm_kernel, addr);
}

/**
 * @}
 * @}
 * @}
 */
