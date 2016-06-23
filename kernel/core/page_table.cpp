#include <page_table.h>
#include <frame_pool.h>

unsigned int PageTable::paging_enabled;
FramePool * PageTable::kernel_mem_pool;
FramePool * PageTable::process_mem_pool;
unsigned long PageTable::shared_size;
PageTable * PageTable::current_page_table;
unsigned long *PageTable::k_page_table;


void PageTable::init_paging(FramePool *_kernel_mem_pool,
		FramePool *_process_mem_pool,
		const unsigned long _shared_size)
{
	paging_enabled = 0;
	kernel_mem_pool = _kernel_mem_pool;
	process_mem_pool = _process_mem_pool;
	shared_size = _shared_size;
}

/* small page translation is used */
PageTable::PageTable(PG_TYPE pagetype) 
{
	int i;
	/* page directory is located in process mem pool */
	if(pagetype == PG_TABLE_USER) {
		page_directory = (unsigned long *)FRAMETOPHYADDR(process_mem_pool->get_frame());
	} else {
		page_directory = (unsigned long *)FRAMETOPHYADDR(kernel_mem_pool->get_frame());
	}
	page_directory = (unsigned long *)((unsigned long)page_directory & 0xffffc000);
	/* set the translation table base */
	__asm__ ("mrc p15, 0, %0, c2, c0, 0" : "=r" (page_directory) ::);

	/* initialize page directory as 0 */
	for(i=0 ; i<4095 ; i++) {
		page_directory[i] = 0;
	}

	if(k_page_table == 0) {
		k_page_table = (unsigned long *)FRAMETOPHYADDR(kernel_mem_pool->get_frame());
		for(i=0 ; i<4 ; i++) {

			/* 0x11 is
			   Bit[1:0] = 01 : 00:fualt, 01:page, 10:section, 11 : reserved
			   Bit[3:2] = 00 : section : B(ufferable), C(achable), page : don't care
			   Bit[4] = 1
			   Bit[8:5] = 0000 : Domain 0
			   Bit[9] = 0 : don't care
			 */
			page_directory[i] = ((unsigned long)k_page_table+(256*i)<<2 | 0x11);
		}
	}

	/* 0x55E
	 * Bit[1:0] = 10 : small page 
	 * Bit[2] = 1 : Bufferable
	 * Bit[3] = 1 : Cacheable
	 * Bit[5:4] = Bit[7:6] = Bit[9:8] = Bit[11:10] = 01 
	 * : subpage access permission,
	 * : 01 only SVC mode can r/w, user mode can't access 
	 */

	for(i=0 ; i<1024 ; i++) {
		k_page_table[i] = ((i*4*(0x1<<10)) | 0x55E;
	}
}

void PageTable::load()
{

}

void PageTable::enable_paging()
{

}

void PageTable::free_page(unsigned long pageAddr)
{

}
