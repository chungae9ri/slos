/* SPDX-License-Identifier: MIT OR Apache-2.0
 *
 * Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>
 */

/**
 * @addtogroup kernel
 * @{
 * @addtogroup kernel_core Core
 * @{
 * @addtogroup kernel_core_mm Memory management
 * @{
 *
 * @brief Kernel virtual memory pool management
 *
 */

#ifndef _VM_POOL_H_
#define _VM_POOL_H_

#include <frame_pool.h>
#include <page_table.h>

/** Virtual memory region descriptor structure */
struct region_desc {
	uint32_t startAddr;
	uint32_t size;
	struct region_desc *next;
	struct region_desc *prev;
};

/** Virtual memory pool */
struct vmpool {
	uint32_t base_address;
	uint32_t size;
	struct region_desc *plast_region;
	uint32_t region_num;
	uint32_t region_size_total;
	struct pagetable *ppagetable;
};

/**
 * @brief Initialize virtual memory pool
 *
 * @param [in] pvmpool Pointer to virtual memory pool
 * @param [in] _pagetable Pointer to page table
 * @param [in] _base_address Base address of virtual memory region
 * @param [in] _size Virtual memory region size
 */
void init_vmpool(struct vmpool *pvmpool, struct pagetable *_pagetable, uint32_t _base_address,
		 uint32_t _size);

/**
 * @brief Allocate memory from virtual memory pool
 *
 * This is a lazy allocate that real allocation occurs when page fault happens
 *
 * @param [in] pvmpool Pointer to virtual memory pool
 * @param [in] _size Size allocated
 * @return uint32_t Address of allocated memory regioin
 */
uint32_t allocate(struct vmpool *pvmpool, uint32_t _size);

/**
 * @brief Free allocated memory
 *
 * @param [in] pvmpool Pointer to virtual memory pool
 * @param [in] _start_address Start address of memory region to be freed
 */
void release(struct vmpool *pvmpool, uint32_t _start_address);

#endif

/**
 * @}
 * @}
 * @}
 *
 */
