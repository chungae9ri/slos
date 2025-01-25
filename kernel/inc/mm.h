/* SPDX-License-Identifier: MIT OR Apache-2.0
 *
 * Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>
 */

#ifndef __MM_H__
#define __MM_H__
#include <stdint.h>
#include <stddef.h>

#include <defs.h>
#include <frame_pool.h>
#include <page_table.h>
#include <vm_pool.h>

extern int __kernel_heap_start__;
extern int __kernel_heap_end__;

struct mm_struct {
	struct framepool kfp;
	struct pagetable pgt;
	struct vmpool heap;
};
void init_kernmem(struct framepool *kfp, struct pagetable *pgt, struct vmpool *kheap);
__section("PGT_INIT") void init_pgt(void);
void *kmalloc(size_t size);
void kfree(uint32_t addr);
#endif
