// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

/**
 * @addtogroup kernel
 * @{
 * @addtogroup kernel_drivers Drivers
 * @{
 * @addtogroup kernel_drivers_dma DMA driver
 * @{
 *
 * @file
 *
 * @brief Driver for DMA peripheral implemented in FPGA
 *
 */

/** Modecore DMA compatible string in devicetree */
#define DEVICE_DT_COMPAT SLOS_MODCORE_DMA

#include <stddef.h>
#include <stdbool.h>

#include <error.h>
#include <dma.h>
#include <regops.h>
#include <gic_v1.h>
#include <mm.h>
#include <printk.h>
#include <generated_devicetree_defs.h>
#include <device.h>
#include <ops.h>
#include <task.h>

/* modcore dma device regs offset */
#define MODCORE_DMA_CNTL_OFFSET	    0x0
#define MODCORE_DMA_STATUS_OFFSET   0x4
#define MODCORE_DMA_SRC_ADDR_OFFSET 0x8
#define MODCORE_DMA_LEN_OFFSET	    0xc
#define MODCORE_DMA_DST_ADDR_OFFSET 0x10

/* modcore status bit mask */
#define BM_MODCORE_DMA_START	0x1
#define BM_MODCORE_DMA_IRQ_DONE 0x2
#define BM_MODCORE_DMA_RESET	0x4

#define MODCORE_DMA_BURST_LEN 0x1000

DEVICE_DEFINE_IDX(dma, 0);

struct dma_work_order {
	uint32_t order_num;
	uint32_t src;
	uint32_t dst;
	uint32_t len;
	struct dma_work_order *next;
	bool is_first;
};

int32_t init_dma(struct device *dev)
{
	uint32_t cntl;

	if (dev == NULL) {
		return -EINVAL;
	}

	dev->name = DT_GET_COMPAT(0);
	dev->base_addr = DT_GET_BASE_ADDR(0);
	dev->irq = DT_GET_IRQ(0);
	dev->data = NULL;

	/* reset mdcore hw */
	cntl = BM_MODCORE_DMA_RESET;
	write32(dev->base_addr + MODCORE_DMA_CNTL_OFFSET, cntl);

	return 0;
}

int32_t set_dma_work(struct device *dev, uint32_t src, uint32_t dst, uint32_t len)
{
	int i, q, r;
	struct dma_work_order *pcur, *ptemp;

	if (dev == NULL) {
		return -EINVAL;
	}

	((struct dma_work_order *)dev)->is_first = true;

	if (len > MODCORE_DMA_BURST_LEN) {
		q = (int)(len / MODCORE_DMA_BURST_LEN);
		r = len % MODCORE_DMA_BURST_LEN;

		for (i = 0; i < q; i++) {
			ptemp = kmalloc(sizeof(struct dma_work_order));
			ptemp->order_num = i;
			ptemp->src = src + MODCORE_DMA_BURST_LEN * i;
			ptemp->dst = dst + MODCORE_DMA_BURST_LEN * i;
			ptemp->len = MODCORE_DMA_BURST_LEN;
			ptemp->next = NULL;

			if (i == 0) {
				dev->data = pcur = ptemp;
			} else {
				pcur->next = ptemp;
				pcur = pcur->next;
			}
		}

		if (r) {
			ptemp = kmalloc(sizeof(struct dma_work_order));
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
		ptemp = kmalloc(sizeof(struct dma_work_order));
		ptemp->order_num = 0;
		ptemp->src = src;
		ptemp->dst = dst;
		ptemp->len = len;
		ptemp->next = NULL;

		dev->data = ptemp;
	}

	return 0;
}

int32_t start_dma(struct device *dev)
{
	uint32_t cntl, src, dst, len;
	struct dma_work_order *ptemp;

	if (dev == NULL) {
		return -EINVAL;
	}

	if (((struct dma_work_order *)(dev->data))->is_first) {
		flush_ent_dcache();
		((struct dma_work_order *)(dev->data))->is_first = false;
	}

	src = ((struct dma_work_order *)(dev->data))->src;
	dst = ((struct dma_work_order *)(dev->data))->dst;
	len = ((struct dma_work_order *)(dev->data))->len;
	printk("dma start, src: 0x%x, dst: 0x%x, 0x%xbytes\n", src, dst, len);

	write32(dev->base_addr + MODCORE_DMA_SRC_ADDR_OFFSET, src);
	write32(dev->base_addr + MODCORE_DMA_DST_ADDR_OFFSET, dst);
	write32(dev->base_addr + MODCORE_DMA_LEN_OFFSET, len);

	ptemp = (struct dma_work_order *)(dev->data);
	dev->data = ((struct dma_work_order *)(dev->data))->next;
	kfree((uint32_t)ptemp);

	cntl = read32(dev->base_addr + MODCORE_DMA_CNTL_OFFSET);
	cntl |= BM_MODCORE_DMA_START;
	write32(dev->base_addr + MODCORE_DMA_CNTL_OFFSET, cntl);

	return 0;
}

int32_t dma_irq(void *arg)
{
	uint32_t cntl;
	struct device *dev = (struct device *)arg;

	if (dev == NULL) {
		return -EINVAL;
	}

	if (dev->data != NULL) {
		printk("enqueue next dma work\n");
		enqueue_workq((int32_t (*)(void *))(start_dma), (void *)dev);
	} else {
		cntl = read32(dev->base_addr + MODCORE_DMA_CNTL_OFFSET);
		cntl &= ~BM_MODCORE_DMA_START;
		write32(dev->base_addr + MODCORE_DMA_CNTL_OFFSET, cntl);
		printk("dma done!\n");
	}

	cntl = read32(dev->base_addr + MODCORE_DMA_CNTL_OFFSET);
	cntl |= BM_MODCORE_DMA_IRQ_DONE;
	write32(dev->base_addr + MODCORE_DMA_CNTL_OFFSET, cntl);

	return 0;
}

/**
 * @}
 * @}
 * @}
 *
 */
