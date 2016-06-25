#ifndef _page_table_H_ 
#define _page_table_H_

#include <frame_pool.h>

typedef enum {PG_TABLE_KERN = 0, PG_TABLE_USER = 1} PG_TYPE;

class PageTable {
	private:
		static PageTable     *current_page_table; /* pointer to currently loaded page table object */
		static unsigned int    paging_enabled;     /* is paging turned on (i.e. are addresses logical)? */
		static FramePool     *kernel_mem_pool;    /* Frame pool for the kernel memory */
		static FramePool     *process_mem_pool;   /* Frame pool for the process memory */
		static unsigned long   shared_size;        /* size of shared address space */

		/* data for current page table */
		unsigned long        *page_directory;     /* where is page directory located? */
		/* static page_table for shared memory(4MB kernel) */
		static unsigned long 		*k_page_table;
		/* kernel task should have common page directory */
		static unsigned long		*k_page_dir;

	public:
		static const unsigned int PAGE_SIZE        ;//= Machine::PAGE_SIZE; /* in bytes */
		static const unsigned int ENTRIES_PER_PAGE ;//= Machine::PT_ENTRIES_PER_PAGE; /* in entries, duh! */

		/* Set the global parameters for the paging subsystem. */
		static void init_paging(FramePool * _kernel_mem_pool,
				FramePool * _process_mem_pool,
				const unsigned long _shared_size);
		/* Initializes a page table with a given location for the directory and the
		   page table proper.
		   The PageTable object still needs to be stored somewhere! Probably it is best
		   to have it on the stack, as there is no memory manager yet...
		   It may also be simpler to create the first page table *before* paging
		   has been enabled.
		 */

		PageTable(PG_TYPE pagetype);

		unsigned long *getpd() {
			return page_directory;
		}
		
		/* Makes the given page table the current table. This must be done once during
		   system startup and whenever the address space is switched (e.g. during
		   process switching). 
		 */
		void load();

		/* Enable paging on the CPU. Typically, a CPU start with paging disabled, and
		   memory is accessed by addressing physical memory directly. After paging is
		   enabled, memory is addressed logically. 
		 */
		static void enable_paging();

		/* page fault handler*/
		static void handle_fault();

		/* Release the frame associated with the page _page_no */
		void free_page(unsigned long _page_no);
};
#endif

