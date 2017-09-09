#include <stdlib.h>
#include <stdint.h>

extern int __kernel_heap_start__;
extern int __kernel_heap_end__;


void *kmalloc(uint32_t size)
{
	static uint8_t *heap = NULL;
	uint8_t *prev_heap;

	if (!heap) {
		heap = (uint8_t *)(&__kernel_heap_start__);
	}

	prev_heap = heap;
	if ((int)(heap + size) >= (int)(&__kernel_heap_end__)) {
		return 0;
	}
	heap += size;
	return (void *) prev_heap;
}

void kfree(uint32_t addr)
{

}
