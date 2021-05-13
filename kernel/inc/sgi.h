#ifndef _SGI_H_
#define _SGI_H_
#include <stdint-gcc.h>

struct sgi_data {
	uint32_t num;
	uint32_t cpuid;
};

int sgi_irq(void *arg);
void enable_sgi_irq(int vec, int(*)(void *arg));
#endif
