#include <stdlib.h>

extern int __heap_start__;
extern int __heap_end__;

/* Simple memory management routine.
   1. without _sbrk, data abort happens while running. libc library might use this.
   2. fix linking error. */
void *_sbrk(void *reent, size_t incr)
{
	static unsigned char *heap = NULL;
	unsigned char *prev_heap;

	if (!heap) {
		heap = (unsigned char *)&__heap_start__;
	}

	prev_heap = heap;
	if ((heap + incr) >= (unsigned char *)&__heap_end__) {
		print_msg("\r\nheap overflow!!");
		return 0;
	}
	heap += incr;
	return (void *) prev_heap;
}
