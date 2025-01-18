// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

#ifndef _PAGE_TABLE_H_
#define _PAGE_TABLE_H_

#include <frame_pool.h>
#include <vm_pool.h>

typedef enum {
	PG_TABLE_KERN = 0,
	PG_TABLE_USER = 1
} PG_TYPE;

struct pagetable {
	/* is paging turned on (i.e. are addresses logical)? */
	unsigned int paging_enabled;
	/* Frame pool for the kernel memory */
	struct framepool *pframepool;
	/* size of shared address space between kernel and user */
	unsigned int shared_size;

	/* static page_table for shared memory(4MB kernel) */
	unsigned int *ppage_table;
	/* kernel task should have common page directory */
	unsigned int *ppage_dir;

	struct vm_pool *pVMref[10];
	int VMcnt;
};

/* Set the global parameters for the paging subsystem. */
void init_pageregion(struct pagetable *ppagetable, struct framepool *pframepool,
                     const unsigned int _shared_size);

void init_pagetable(struct pagetable *ppagetable, PG_TYPE pagetype);
void load_pagetable(struct pagetable *ppagetable);

/* page fault handler*/
void handle_fault();

/* Release the frame associated with the page _page_no */
void free_page(unsigned int _page_no);
#if 0
void register_vmpool(VMPool *_pool);
#endif
#endif
