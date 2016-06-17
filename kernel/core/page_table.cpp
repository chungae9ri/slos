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

PageTable::PageTable() 
{

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
