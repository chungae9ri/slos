#ifndef _page_table_H_ 
#define _page_table_H_

#include <frame_pool.h>
#include <vm_pool.h>

typedef enum {PG_TABLE_KERN = 0, PG_TABLE_USER = 1} PG_TYPE;

struct pagetable { 
	/* is paging turned on (i.e. are addresses logical)? */
	unsigned int paging_enabled;
	/* Frame pool for the kernel memory */
	struct framepool	*kernel_framepool;    
	/* Frame pool for the process memory */
	struct framepool *process_framepool;   
	/* size of shared address space */
	unsigned int shared_size; 

	/* data for current page table */
	/* where is page directory located? */
	unsigned int *page_directory;
	/* static page_table for shared memory(4MB kernel) */
	unsigned int *k_page_table;
	/* kernel task should have common page directory */
	unsigned int *k_page_dir;

	struct vm_pool *pVMref[10];
	int VMcnt;
};

/* Set the global parameters for the paging subsystem. */
void init_pageregion(struct pagetable *ppagetable,
		     struct framepool *pkernel_framepool,
		     struct framepool *pprocess_framepool,
		     const unsigned int _shared_size);

void init_pagetable(struct pagetable *ppagetable, PG_TYPE pagetype);

void load_pagetable(struct pagetable *ppagetable);
void enable_paging();

/* page fault handler*/
void handle_fault();

/* Release the frame associated with the page _page_no */
void free_page(unsigned int _page_no);
#if 0
void register_vmpool(VMPool *_pool);
#endif
#endif
