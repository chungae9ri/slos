#include <vm_pool.h>
#include <page_table.h>

#define PAGE_SIZE	(4*1024)

VMPool::VMPool(unsigned int _base_address,
		unsigned int _size,
		FramePool *_frame_pool,
		PageTable *_page_table)
{
	base_address = _base_address;
	size = _size;
	frame_pool = _frame_pool;
	page_table = _page_table;

	page_table->register_vmpool(this);

	/* alloc a frame for region descriptor
	 * to do this, generate a page fault 
	 * at the base address
	 */
	pcur_region = (struct region_desc *)base_address;
	/* generate a page fault 
	 * set the bit[0] with 1 as it is allocated 
	 * for region descriptor
	 */
	pcur_region->startAddr = base_address;
	pcur_region->size = PAGE_SIZE;
	pcur_region->prev = 0;
	pcur_region->next = 0;
	region_page_total = PAGE_SIZE;
}

/* lazy allocator */
unsigned int VMPool::allocate(unsigned int _size)
{
	unsigned int pgnum;

	/* allocate multiple of page, internal fragmentation allowed */
	pgnum = (int)(_size/PAGE_SIZE) + (_size%PAGE_SIZE ? 1 : 0);

	if(!pcur_region) {
		pcur_region = (struct region_desc *)base_address;
		while(pcur_region->next) {
			pcur_region = pcur_region->next; 
		}
	}

	pcur_region->next = pcur_region + 1;
	pcur_region->next->prev = pcur_region;
	pcur_region = pcur_region->next;
	pcur_region->startAddr = base_address + region_page_total;
	pcur_region->size = _size;
	pcur_region->next = 0;

	/* always allocate multiples of page size */
	region_page_total += (PAGE_SIZE * pgnum);

	return pcur_region->startAddr;
}

void VMPool::release(unsigned int _start_address)
{
	int i;
	unsigned int region_size;
	unsigned int pgnum;
	struct region_desc *pcur = (struct region_desc *)(base_address);

	while(pcur) {
		if(pcur->startAddr == _start_address) {
			pcur->prev->next = pcur->next;
			if(pcur->next) 
				pcur->next->prev = pcur->prev;
			region_size = pcur->size;
			pgnum = (int)(region_size/PAGE_SIZE) + (region_size%PAGE_SIZE ? 1 : 0);
			for(i=0 ; i<pgnum ; i++) {
				page_table->free_page((unsigned int)(_start_address+(PAGE_SIZE*i)));
			}
			if(i == pgnum) {
				/* flushing TLB and reload page table */
				page_table->load();
				region_page_total -= (pgnum * PAGE_SIZE);
			}
			break;
		}
		pcur = pcur->next;
	}

	if(pcur_region->startAddr == _start_address) {
		pcur_region = (struct region_desc *)(base_address);
		while(pcur_region->next) {
			pcur_region = pcur_region->next; 
		}
	}
}

bool VMPool::is_legitimate(unsigned int _address)
{
	struct region_desc *pcur = (struct region_desc *)(base_address);

	if(_address == base_address) return true;

	pcur = pcur->next;
	while(pcur) {
		if(_address >= pcur->startAddr && _address < pcur->startAddr + pcur->size) return true;
		else pcur = pcur->next;
	}

	return false;
}
