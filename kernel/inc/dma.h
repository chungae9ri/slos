// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

#include <stdint.h>
#include <mem_layout.h>

#define MODCORE_DMA_IRQ_ID		61U
#define MODCORE_DMA_START		0x1
#define MODCORE_DMA_IRQ_DONE		0x2
#define MODCORE_RESET			0x4

extern void flush_ent_dcache(void);

struct dma_work_order {
	uint32_t order_num;
	uint32_t src;
	uint32_t dst;
	uint32_t len;
	struct dma_work_order *next;
};

void init_dma(void);
void set_dma_work(uint32_t src, uint32_t dst, uint32_t len);
void start_dma(void *arg);
int dma_irq (void *arg);
