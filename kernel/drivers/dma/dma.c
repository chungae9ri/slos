// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

#include <stddef.h>

#include <dma.h>
#include <regops.h>
#include <gic_390.h>
#include <mm.h>
#include <printk.h>
#include <generated_devicetree_defs.h>
#include <device.h>

/* modcore dma device regs offset */
#define MODCORE_DMA_CNTL_OFFSET			0x0
#define MODCORE_DMA_STATUS_OFFSET		0x4
#define MODCORE_DMA_SRC_ADDR_OFFSET		0x8
#define MODCORE_DMA_LEN_OFFSET			0xc
#define MODCORE_DMA_DST_ADDR_OFFSET		0x10

/* modcore status bit mask */
#define BM_MODCORE_DMA_START			0x1
#define BM_MODCORE_DMA_IRQ_DONE			0x2
#define BM_MODCORE_DMA_RESET			0x4

#define MODCORE_DMA_BURST_LEN			0x1000

DEVICE_DEFINE(dma,
			  DT_N_S_dma_43c10000_P_compat,
			  DT_N_S_dma_43c10000_P_base_addr,
			  DT_N_S_dma_43c10000_P_intr);

struct dma_work_order {
	uint32_t order_num;
	uint32_t src;
	uint32_t dst;
	uint32_t len;
	struct dma_work_order *next;
};

static struct dma_work_order *p_dma_work_order;
int bFirst;
uint32_t enqueue_workq(void (*func)(void *), void *arg);
extern void flush_ent_dcache(void);

void init_dma(void)
{
	uint32_t cntl;

	gic_register_int_handler(DEVICE_GET_IRQ(dma), dma_irq, NULL);
	gic_mask_interrupt(DEVICE_GET_IRQ(dma));
	p_dma_work_order = NULL;
	/* reset mdcore hw */
	cntl = BM_MODCORE_DMA_RESET;
	write32(DEVICE_GET_BASE_ADDR(dma) + MODCORE_DMA_CNTL_OFFSET, cntl);
}

void set_dma_work(uint32_t src, uint32_t dst, uint32_t len)
{
	int i, q, r;
	struct dma_work_order *pcur, *ptemp;

	bFirst = 1;

	if (len > MODCORE_DMA_BURST_LEN) {
		q = (int)(len / MODCORE_DMA_BURST_LEN);
		r = len % MODCORE_DMA_BURST_LEN;

		for (i = 0; i < q; i++) {
			ptemp = (struct dma_work_order *)kmalloc(sizeof(struct dma_work_order));
			ptemp->order_num = i;
			ptemp->src = src + MODCORE_DMA_BURST_LEN * i;
			ptemp->dst = dst + MODCORE_DMA_BURST_LEN * i;
			ptemp->len = MODCORE_DMA_BURST_LEN;
			ptemp->next = NULL;
			
			if (i == 0) {
				p_dma_work_order = pcur = ptemp;
			} else {
				pcur->next = ptemp;
				pcur = pcur->next;
			}
		}

		if (r) {
			ptemp = (struct dma_work_order *)kmalloc(sizeof(struct dma_work_order));
			ptemp->order_num = i;
			ptemp->src = src + MODCORE_DMA_BURST_LEN * i;
			ptemp->dst = dst + MODCORE_DMA_BURST_LEN * i;
			ptemp->len = r;
			ptemp->next = NULL;

			if (pcur) {
				pcur->next = ptemp;
			}
		}
	} else {
		ptemp = (struct dma_work_order *)kmalloc(sizeof(struct dma_work_order));
		ptemp->order_num = 0;
		ptemp->src = src;
		ptemp->dst = dst;
		ptemp->len = len;
		ptemp->next = NULL;

		p_dma_work_order = ptemp;
	}
}

void start_dma(void *arg)
{
	uint32_t cntl, src, dst, len;
	struct dma_work_order *ptemp;

	if (bFirst) {
		flush_ent_dcache();
		bFirst = 0;
	}

	src = p_dma_work_order->src;
	dst = p_dma_work_order->dst;
	len = p_dma_work_order->len;
	printk("dma start: 0x%x, 0x%x, 0x%x\n", src, dst, len);

	write32(DEVICE_GET_BASE_ADDR(dma) + MODCORE_DMA_SRC_ADDR_OFFSET, src);
	write32(DEVICE_GET_BASE_ADDR(dma) + MODCORE_DMA_DST_ADDR_OFFSET, dst);
	write32(DEVICE_GET_BASE_ADDR(dma) + MODCORE_DMA_LEN_OFFSET, len);

	ptemp = p_dma_work_order;
	p_dma_work_order = p_dma_work_order->next;
	kfree((uint32_t)ptemp);

	cntl = read32(DEVICE_GET_BASE_ADDR(dma) + MODCORE_DMA_CNTL_OFFSET);
	cntl |= BM_MODCORE_DMA_START;
	write32(DEVICE_GET_BASE_ADDR(dma) + MODCORE_DMA_CNTL_OFFSET, cntl);
}

int dma_irq (void *arg)
{
	uint32_t cntl;
	if (p_dma_work_order) {
		printk("enqueue next dma work\n");
		enqueue_workq(start_dma, NULL);
	} else {
		cntl = read32(DEVICE_GET_BASE_ADDR(dma) + MODCORE_DMA_CNTL_OFFSET);
		cntl &= ~BM_MODCORE_DMA_START;
		write32(DEVICE_GET_BASE_ADDR(dma) + MODCORE_DMA_CNTL_OFFSET, cntl);
		printk("dma done!\n");
	}

	cntl = read32(DEVICE_GET_BASE_ADDR(dma) + MODCORE_DMA_CNTL_OFFSET);
	cntl |= BM_MODCORE_DMA_IRQ_DONE;
	write32(DEVICE_GET_BASE_ADDR(dma) + MODCORE_DMA_CNTL_OFFSET, cntl);

	return 0;
}
