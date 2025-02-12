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
 * @brief Kernel memory management module
 *
 */

#ifndef __MM_H__
#define __MM_H__
#include <stdint.h>
#include <stddef.h>

#include <defs.h>
#include <frame_pool.h>
#include <page_table.h>
#include <vm_pool.h>

/** Kernel heap start address, imported from linker script */
extern int __kernel_heap_start__;
/** Kernel heap end address, imported from linker script */
extern int __kernel_heap_end__;

/** Memory manager struct */
struct mm_struct {
	struct framepool kfp; /**< Kernel memory frame pool */
	struct pagetable pgt; /**< Page table */
	struct vmpool heap;   /**< Virtual memory pool */
};

/**
 * @brief Initialize kernel memory region
 *
 * @param [in] kfp Pointer to kernel memory frame pool
 * @param [in] pgt Pointer to page table
 * @param [in] kheap Pointer to virtual memory pool for kernel heap region
 */
void init_kernmem(struct framepool *kfp, struct pagetable *pgt, struct vmpool *kheap);

/**
 * @brief Initialize page table
 *
 */
__section("PGT_INIT") void init_pgt(void);

/**
 * @brief Allocate memory from heap
 *
 * @param [in] size Byte size of allocation
 * @return void* Pointer to allocated memory region
 */
void *kmalloc(size_t size);

/**
 * @brief Free allocated memory from heap
 *
 * @param [in] addr Address of freed memory region
 */
void kfree(uint32_t addr);

#endif

/**
 * @}
 * @}
 * @}
 *
 */
