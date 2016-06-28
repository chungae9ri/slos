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
int PageTable::VMcnt;
VMPool *PageTable::pVMref[10];


void PageTable::init_paging(FramePool *_kernel_mem_pool,
		FramePool *_process_mem_pool,
		const unsigned int _shared_size)
{
	paging_enabled = 0;
	kernel_mem_pool = _kernel_mem_pool;
	process_mem_pool = _process_mem_pool;
	shared_size = _shared_size;
	k_page_dir = 0;
	VMcnt = 0;
}

/* small page translation is used */
PageTable::PageTable(PG_TYPE pagetype) 
{
	int i, j;
	
	/* all kernel tasks share the same page directory */
	if(pagetype == PG_TABLE_KERN && k_page_dir)
		return;

	/* page directory is located in process mem pool 
	 * 16KB for 4096 entry(16KB = 4K * 4Byte) is needed
	 */
	if(pagetype == PG_TABLE_USER) {
		page_directory = (unsigned int *)FRAMETOPHYADDR(process_mem_pool->get_frame());
		/* alloc 3 more contiguous page frames */
		for(i=0 ; i<3 ; i++) {
			FRAMETOPHYADDR(process_mem_pool->get_frame());
		}
	} else {
		page_directory = (unsigned int *)FRAMETOPHYADDR(kernel_mem_pool->get_frame());
		/* alloc 3 more contiguous page frames */
		for(i=0 ; i<3 ; i++) {
			FRAMETOPHYADDR(kernel_mem_pool->get_frame());
		}
	}
	page_directory = (unsigned int *)((unsigned int)page_directory & 0xffffc000);

	/* initialize page directory as 0 */
	for(i=0 ; i<4095 ; i++) {
		page_directory[i] = 0;
	}

	if(k_page_table == 0) {
		/* 16MB(=4MB *4) directly mapped memory
		 * 4MB ~ 16MB kernel heap
		 */
		for(i=0 ; i<4 ; i++) {
			k_page_table = (unsigned int *)FRAMETOPHYADDR(kernel_mem_pool->get_frame());
			for(j=0 ; j<4 ; j++) {

				/* 0x11 is
				   Bit[1:0] = 01 : 00:fualt, 01:page, 10:section, 11 : reserved
				   Bit[3:2] = 00 : section : B(ufferable), C(achable), page : don't care
				   Bit[4] = 1
				   Bit[8:5] = 0000 : Domain 0
				   Bit[9] = 0 : don't care
				 */
				page_directory[i*4+j] = ((unsigned int)k_page_table+(256*j)<<2 | 0x11);
			}
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

	/* 16MB(4 * 4KB page * 1024 entry) direct mapped memory */
	for(i=0 ; i<4 ; i++) {
		k_page_table = (unsigned int *)(page_directory[i*4]);
		for(j=0 ; j<1024 ; j++) {
			k_page_table[j] = (j*4*(0x1<<10)) | 0x55E;
		}
	}

	if(pagetype == PG_TABLE_KERN) {
		k_page_dir = page_directory;
	}
}

void PageTable::load()
{
	current_page_table = this;

	/* write the translation table base */
	asm ("mcr p15, 0, %0, c2, c0, 0" : : "r" (current_page_table->page_directory) :);
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
	asm("mcr p15, 0, %0, c1, c0, 0" : : "r" (c1):);
}

void PageTable::handle_fault()
{
	int i;
	unsigned int *pda, *pta;
	unsigned int *pfa;
	unsigned int *pde, *pte;
	unsigned int *page_table, *frame_addr;

	/* read DFAR */
	asm volatile ("mrc p15, 0, %0, c6, c0, 0" : "=r" (pfa) ::);
	/* read ttb */
	asm volatile ("mrc p15, 0, %0, c2, c0, 0" : "=r" (pda) ::);

	/* check fault address is in valid VM region */
	for(i=0 ; i<VMcnt ; i++) {
		if(pVMref[i]->is_legitimate((unsigned int)pfa)) break;
	}

	/* the fault address is not in valid VM region */
	if(VMcnt != 0 && i == VMcnt) {
		return;
	}
	/* entry for 1st level descriptor */
	pde = (unsigned int *)((0xffffc000 & *pda) | ((0xfff00000 & *pfa)>>18));

	if(*pde == 0x0) {
		page_table = (unsigned int *)FRAMETOPHYADDR(process_mem_pool->get_frame());
		for(i=0 ; i<4 ; i++) {
			/* set the value of 1st level descriptor */
			*pde = (unsigned int)((unsigned int)page_table + (256*i)<<2 | 0x11); 
		}
	}

	frame_addr = (unsigned int *)FRAMETOPHYADDR(process_mem_pool->get_frame());

	/* entry for 2nd level descriptor */
	pte = (unsigned int *)((*pde & 0xfffffc00) | ((0x000ff000 & *pfa)>>22));
	/* set the value of 2nd level descriptor */
	*pte = ((unsigned int)frame_addr | 0x55E);
}

void PageTable::register_vmpool(VMPool *_pool)
{
	pVMref[VMcnt++] = _pool;
}

void PageTable::free_page(unsigned int pageAddr)
{
	unsigned int *pda, *pde, *pte;
	unsigned int *frame_addr;
	unsigned int frame_num, frame_num_k_heap;
	/* read ttb */
	asm volatile ("mrc p15, 0, %0, c2, c0, 0" : "=r" (pda) ::);
	/* get the 1st level descriptor */
	pde = (unsigned int *)((*pda & 0xfffc0000) | ((pageAddr & 0xfff00000)>>18));
	pte = (unsigned int *)((*pde & 0xfffffc00) | ((pageAddr & 0x000ff000)>>22));

	/* physical address of frame */
	frame_addr = (unsigned int *)(*pte);
	frame_num = (unsigned int)(frame_addr) >> 12;

	if(frame_num >= KERNEL_HEAP_START_FRAME &&
 	   frame_num < KERNEL_HEAP_START_FRAME + KERNEL_HEAP_FRAME_NUM) {
		kernel_mem_pool->release_frame(frame_num);
	} else if(frame_num >= PROCESS_HEAP_START_FRAME &&
		  frame_num < PROCESS_HEAP_START_FRAME + PROCESS_HEAP_FRAME_NUM) {
		process_mem_pool->release_frame(frame_num);
	} else {
		/* error !
		 * only frames in heap can be freed.
		 */
	}

	*pte = 0x0;
}
