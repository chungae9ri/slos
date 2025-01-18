// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

#ifndef _VM_POOL_H_ // include file only once
#define _VM_POOL_H_

#include <frame_pool.h>
#include <page_table.h>

/* region descriptor structure */
struct region_desc {
	unsigned int startAddr;
	unsigned int size;
	struct region_desc *next;
	struct region_desc *prev;
};

/* Virtual Memory Pool */
struct vmpool {
	unsigned int base_address;
	unsigned int size;
	struct region_desc *plast_region;
	unsigned int region_num;
	unsigned int region_size_total;
	struct pagetable *ppagetable;
};

void init_vmpool(struct vmpool *pvmpool, struct pagetable *_pagetable, unsigned int _base_address,
                 unsigned int _size);

unsigned int allocate(struct vmpool *pvmpool, unsigned int _size);
void release(struct vmpool *pvmpool, unsigned int _start_address);
int is_legitimate(struct vmpool *pvmpool, unsigned int _address);

#endif
