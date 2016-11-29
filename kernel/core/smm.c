#include <stdlib.h>
#include <vm_pool.h>

extern int __kernel_heap_start__;
extern int __kernel_heap_end__;

#ifdef USE_MMU
extern struct vmpool *pvm_kernel;
extern struct vmpool *pvm_user;
#endif

extern int print_msg(const char *str);
/* Simple memory management routine.
   1. without _sbrk, data abort happens while running. libc library might use this.
   2. fix linking error. 
 */
#ifdef USE_MMU
void *_sbrk(void *reent, unsigned int incr)
{
	return (void *)(allocate(pvm_user, incr));
}
#endif

void *_sbrk(void *reent, unsigned int size)
{
#ifdef USE_MMU
	return (void *)(allocate(pvm_kernel, size));
#else
	static unsigned char *heap = NULL;
	unsigned char *prev_heap;
	if (!heap) {
		/*heap = (unsigned char *)__kernel_heap_start__;*/
		heap = (unsigned char *)0x800000;
	}

	prev_heap = heap;
	/*if ((heap + size) >= __kernel_heap_end__) {*/
	if ((heap + size) >= (unsigned char *)0x1000000) {
		print_msg("heap overflow!!\n");
		return 0;
	}
	heap += size;
	return (void *) prev_heap;
#endif
}

void *kmalloc(unsigned int size)
{
#ifdef USE_MMU
	return (void *)(allocate(pvm_kernel, size));
#else
	static unsigned char *heap = NULL;
	unsigned char *prev_heap;
	if (!heap) {
		/*heap = (unsigned char *)__kernel_heap_start__;*/
		heap = (unsigned char *)0x800000;
	}

	prev_heap = heap;
	/*if ((heap + size) >= __kernel_heap_end__) {*/
	if ((heap + size) >= (unsigned char *)0x1000000) {
		print_msg("heap overflow!!\n");
		return 0;
	}
	heap += size;
	return (void *) prev_heap;
#endif
}

void kfree(unsigned int free_addr)
{
#ifdef USE_MMU
	release(pvm_kernel, free_addr);
#else
#endif
}
void free(void *free_addr)
{
#ifdef USE_MMU
	release(pvm_user, (unsigned int)free_addr);
#endif
}
