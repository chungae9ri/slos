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
 * @brief Virtual memory pool management functions
 *
 */

#include <vm_pool.h>
#include <page_table.h>
#include <printk.h>

/** Page size 4KiB */
#define PAGE_SIZE (4 * 1024)
/** Bit shift amount for page size */
#define PAGE_SIZE_SHIFT (12)
/** Page size mask */
#define PAGE_SIZE_MASK (0x00000FFF)

void init_vmpool(struct vmpool *pvmpool, struct pagetable *_pagetable, unsigned int _base_address,
		 unsigned int _size)
{
	int region_num_supported;

	pvmpool->ppagetable = _pagetable;
	pvmpool->base_address = _base_address;
	pvmpool->size = _size;

	/* Alloc a frame for region descriptor
	 * to do this, generate a page fault
	 * at the base address.
	 * Region descriptor has 4KB size limit.
	 */
	pvmpool->plast_region = (struct region_desc *)(pvmpool->base_address);
	pvmpool->plast_region->startAddr = pvmpool->base_address;
	pvmpool->plast_region->size = PAGE_SIZE;
	pvmpool->plast_region->prev = 0;
	pvmpool->plast_region->next = 0;
	pvmpool->region_size_total = PAGE_SIZE;

	region_num_supported = (int)(PAGE_SIZE / sizeof(struct vmpool));

	printk("Total %d number of kmalloc calls(vm region desc) supported!\n",
	       region_num_supported);
}

/* Lazy allocator */
unsigned int allocate(struct vmpool *pvmpool, unsigned int _size)
{
	unsigned int pgnum;

	/* Allocate multiple of pages, internal fragmentation allowed */
	pgnum = (int)(_size >> PAGE_SIZE_SHIFT) + ((_size & PAGE_SIZE_MASK) ? 1 : 0);

	if (!pvmpool->plast_region) {
		pvmpool->plast_region = (struct region_desc *)(pvmpool->base_address);
		while (pvmpool->plast_region->next) {
			pvmpool->plast_region = pvmpool->plast_region->next;
		}
	}

	pvmpool->plast_region->next = pvmpool->plast_region + 1;
	pvmpool->plast_region->next->prev = pvmpool->plast_region;
	pvmpool->plast_region = pvmpool->plast_region->next;
	pvmpool->plast_region->startAddr = pvmpool->base_address + pvmpool->region_size_total;
	pvmpool->plast_region->size = _size;
	pvmpool->plast_region->next = 0;

	/* Always allocate multiples of page size */
	pvmpool->region_size_total += (PAGE_SIZE * pgnum);

	return pvmpool->plast_region->startAddr;
}

void release(struct vmpool *pvmpool, unsigned int _start_address)
{
	int i;
	unsigned int region_size;
	unsigned int pgnum;
	struct region_desc *pcur = (struct region_desc *)(pvmpool->base_address);

	while (pcur) {
		if (pcur->startAddr == _start_address) {
			pcur->prev->next = pcur->next;
			if (pcur->next) {
				pcur->next->prev = pcur->prev;
			}

			region_size = pcur->size;
			pgnum = (int)(region_size >> PAGE_SIZE_SHIFT) +
				((region_size & PAGE_SIZE_MASK) ? 1 : 0);
			for (i = 0; i < pgnum; i++) {
				free_page((unsigned int)(_start_address + (PAGE_SIZE * i)));
			}

			if (i == pgnum) {
				/* Flushing TLB and reload page table */
				load_pagetable(pvmpool->ppagetable);
				pvmpool->region_size_total -= (pgnum * PAGE_SIZE);
			}
			break;
		}
		pcur = pcur->next;
	}

	if (pvmpool->plast_region->startAddr == _start_address) {
		pvmpool->plast_region = (struct region_desc *)(pvmpool->base_address);
		while (pvmpool->plast_region->next) {
			pvmpool->plast_region = pvmpool->plast_region->next;
		}
	}
}

int is_legitimate(struct vmpool *pvmpool, unsigned int _address)
{
	struct region_desc *pcur = (struct region_desc *)(pvmpool->base_address);

	if (_address == pvmpool->base_address) {
		return 1;
	}

	pcur = pcur->next;
	while (pcur) {
		if (_address >= pcur->startAddr && _address < pcur->startAddr + pcur->size) {
			return -1;
		}

		pcur = pcur->next;
	}

	return 0;
}

/**
 * @}
 * @}
 * @}
 *
 */
