/*
  kernel/inc/vm_pool.h 
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

#ifndef _VM_POOL_H_                   // include file only once
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

void init_vmpool(struct vmpool *pvmpool,
                 struct pagetable *_pagetable,
		 unsigned int _base_address,
                 unsigned int _size);

unsigned int allocate(struct vmpool *pvmpool, unsigned int _size);
void release(struct vmpool *pvmpool, unsigned int _start_address);
int is_legitimate(struct vmpool *pvmpool, unsigned int _address);

#endif
