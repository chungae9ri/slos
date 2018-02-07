/*
  kernel/inc/page_table.h 
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
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef _page_table_H_ 
#define _page_table_H_

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
void init_pageregion(struct pagetable *ppagetable,
		     struct framepool *pframepool,
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
