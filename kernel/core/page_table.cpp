#include <page_table.h>
#include <frame_pool.h>

#define ENABLE_MMU		0x00000001
#define ENABLE_DCACHE		0x00000004
#define ENABLE_ICACHE		0x00001000
#define MASK_MMU		0x00000001
#define MASK_DCACHE		0x00000004
#define MASK_ICACHE		0x00001000


unsigned int PageTable::paging_enabled;
FramePool * PageTable::kernel_mem_pool;
FramePool * PageTable::process_mem_pool;
unsigned int PageTable::shared_size;
PageTable * PageTable::current_page_table;
unsigned int *PageTable::k_page_table;
unsigned int *PageTable::k_page_dir;


void PageTable::init_paging(FramePool *_kernel_mem_pool,
		FramePool *_process_mem_pool,
		const unsigned int _shared_size)
{
	paging_enabled = 0;
	kernel_mem_pool = _kernel_mem_pool;
	process_mem_pool = _process_mem_pool;
	shared_size = _shared_size;
	k_page_dir = 0;
}

/* small page translation is used */
PageTable::PageTable(PG_TYPE pagetype) 
{
	int i;
	
	if(pagetype == PG_TABLE_KERN && k_page_dir)
		return;

	/* page directory is located in process mem pool */
	if(pagetype == PG_TABLE_USER) {
		page_directory = (unsigned int *)FRAMETOPHYADDR(process_mem_pool->get_frame());
	} else {
		page_directory = (unsigned int *)FRAMETOPHYADDR(kernel_mem_pool->get_frame());
	}
	page_directory = (unsigned int *)((unsigned int)page_directory & 0xffffc000);

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
		k_page_table[i] = (i*4*(0x1<<10)) | 0x55E;
	}

	if(pagetype == PG_TABLE_KERN) {
		k_page_dir = page_directory;
	}
}

void PageTable::load()
{
	current_page_table = this;

	/* read the translation table base */
	asm ("mrc p15, 0, %0, c2, c0, 0" : "=r" (current_page_table->page_directory) ::);
}

void PageTable::enable_paging()
{
	unsigned int enable = ENABLE_MMU | ENABLE_DCACHE | ENABLE_ICACHE;
	unsigned int mask = MASK_MMU | MASK_DCACHE | MASK_ICACHE;
	unsigned int c1;
	/* read control (c1) register of cp 15*/
	asm ("mrc p15, 0, %0, c1, c0, 0" : "=r" (c1) ::);
	c1 &= ~mask;
	c1 |= enable;
	/* write control register */
	asm("mcr p15, 0, %1, c1, c0, 0" : : "r" (c1):);
}

void PageTable::handle_fault()
{

}

void PageTable::free_page(unsigned int pageAddr)
{

}
