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
 * @brief Page table functions for memory management
 *
 */

#ifndef _PAGE_TABLE_H_
#define _PAGE_TABLE_H_

#include <frame_pool.h>
#include <vm_pool.h>

/** Page table user type PG_TABLE_KERN is only valid */
typedef enum {
	PG_TABLE_KERN = 0,
	PG_TABLE_USER = 1
} PG_TYPE;

/** Page table structure */
struct pagetable {
	/* is paging turned on (i.e. are addresses logical)? */
	unsigned int paging_enabled;
	/* Frame pool for the kernel memory */
	struct framepool *pframepool;
	/* Size of shared address space between kernel and user */
	unsigned int shared_size;

	/* Static page_table for shared memory(4MB kernel) */
	unsigned int *ppage_table;
	/* Kernel task should have common page directory */
	unsigned int *ppage_dir;

	struct vm_pool *pVMref[10];
	int VMcnt;
};

/**
 * @brief Reset page table struct
 *
 * @param [in] ppagetable Pointer to page table struct
 * @param [in] pframepool Pointer to memory frame pool
 * @param [in] _shared_size Shared size
 */
void init_pageregion(struct pagetable *ppagetable, struct framepool *pframepool,
		     const unsigned int _shared_size);

/**
 * @brief Initialize page tables' base addresses
 *
 * @param [in] ppagetable Pointer to page table
 * @param [in] pagetype Page type - user or kernel
 */
void init_pagetable(struct pagetable *ppagetable, PG_TYPE pagetype);

/**
 * @brief Set current page table
 *
 * @param [in] ppagetable Page table pointer to be set to current page table
 */
void load_pagetable(struct pagetable *ppagetable);

/**
 * @brief Page fault handler
 *
 */
void handle_fault(void);

/* Release the frame associated with the page _page_no */

/**
 * @brief Free memory frame and delete entry in the page table
 *
 * @param [in] _page_no Page number to be free-ed
 */
void free_page(unsigned int _page_no);

#endif

/**
 * @}
 * @}
 * @}
 *
 */
