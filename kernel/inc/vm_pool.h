
#ifndef _VM_POOL_H_                   // include file only once
#define _VM_POOL_H_

#include <frame_pool.h>

/* Forward declaration of class PageTable */
class PageTable;

/* Virtual Memory Pool */
class VMPool { 
	private:
		unsigned int base_address;
		unsigned int size;
		/* region descriptor structure */
		struct region_desc {
			unsigned int startAddr;
			unsigned int size;
			struct region_desc *next;
			struct region_desc *prev;
		};
		struct region_desc *pcur_region;
		unsigned int region_num;
		unsigned int region_page_total;
		FramePool *frame_pool;
		PageTable *page_table;

	public:
		VMPool(unsigned int _base_address,
				unsigned int _size,
				FramePool *_frame_pool,
				PageTable *_page_table);

		unsigned int allocate(unsigned int _size);
		void release(unsigned int _start_address);
		bool is_legitimate(unsigned int _address);
};

#endif
