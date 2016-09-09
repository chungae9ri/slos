#include <stdlib.h>
#include <vm_pool.h>

extern int __heap_start__;
extern int __heap_end__;

extern struct vmpool *pvm_kernel;
extern struct vmpool *pvm_user;

/* Simple memory management routine.
   1. without _sbrk, data abort happens while running. libc library might use this.
   2. fix linking error. 
 */
void *_sbrk(void *reent, unsigned int incr)
{
	return (void *)(allocate(pvm_user, incr));
}

void *kmalloc(unsigned int size)
{
	return (void *)(allocate(pvm_kernel, size));
}

void kfree(unsigned int free_addr)
{
	release(pvm_kernel, free_addr);
}
void free(void *free_addr)
{
	release(pvm_user, (unsigned int)free_addr);
}
