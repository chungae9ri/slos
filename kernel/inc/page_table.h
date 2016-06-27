#ifndef _page_table_H_ 
#define _page_table_H_

#include <frame_pool.h>
#include <vm_pool.h>

typedef enum {PG_TABLE_KERN = 0, PG_TABLE_USER = 1} PG_TYPE;

class PageTable {
	private:
		static PageTable     *current_page_table; /* pointer to currently loaded page table object */
		static unsigned int    paging_enabled;     /* is paging turned on (i.e. are addresses logical)? */
		static FramePool     *kernel_mem_pool;    /* Frame pool for the kernel memory */
		static FramePool     *process_mem_pool;   /* Frame pool for the process memory */
		static unsigned int shared_size;        /* size of shared address space */

		/* data for current page table */
		unsigned int *page_directory;     /* where is page directory located? */
		/* static page_table for shared memory(4MB kernel) */
		static unsigned int *k_page_table;
		/* kernel task should have common page directory */
		static unsigned int *k_page_dir;

		static VMPool *pVMref[10];
		static int VMcnt;
	public:
		/* Set the global parameters for the paging subsystem. */
		static void init_paging(FramePool * _kernel_mem_pool,
				FramePool * _process_mem_pool,
				const unsigned int _shared_size);

		PageTable(PG_TYPE pagetype);

		unsigned int *getpd() {
			return page_directory;
		}
		
		void load();
		static void enable_paging();

		/* page fault handler*/
		static void handle_fault();

		/* Release the frame associated with the page _page_no */
		void free_page(unsigned int _page_no);
  		void register_vmpool(VMPool *_pool);
};
#endif
