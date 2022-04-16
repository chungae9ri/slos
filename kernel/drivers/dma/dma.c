/*
  kernel/drivers/dma.c dma coprocessor driver
  (C) 2018 Kwangdo Yi <kwangdo.yi@gmail.com>
 
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, see <http://www.gnu.org/licenses/>
*/
#include <stddef.h>
#include <dma.h>
#include <regops.h>
#include <gic.h>
#include <mm.h>
#include <printk.h>

#define MODCORE_DMA_BURST_LEN	0x1000

struct dma_work_order *p_dma_work_order;
int bFirst;
uint32_t enqueue_workq(void (*func)(void *), void *arg) ;

void init_dma(void)
{
	uint32_t cntl;

	gic_register_int_handler(MODCORE_DMA_IRQ_ID, dma_irq, NULL);
	gic_mask_interrupt(MODCORE_DMA_IRQ_ID);
	p_dma_work_order = NULL;
	// reset mdcore hw
	cntl = MODCORE_RESET;
	writel(cntl, MODCORE_DMA_REG_CNTL);
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
		ptemp->len = MODCORE_DMA_BURST_LEN;
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

	writel(src, MODCORE_DMA_REG_SRC_ADDR);
	writel(dst, MODCORE_DMA_REG_DST_ADDR);
	writel(len, MODCORE_DMA_REG_LEN);

	ptemp = p_dma_work_order;
	p_dma_work_order = p_dma_work_order->next;
	kfree((uint32_t)ptemp);

	cntl = readl(MODCORE_DMA_REG_CNTL);
	cntl |= MODCORE_DMA_START;
	writel(cntl, MODCORE_DMA_REG_CNTL);
}

int dma_irq (void *arg)
{
	uint32_t cntl;
	if (p_dma_work_order) {
		enqueue_workq(start_dma, NULL);
	} else {
		cntl = readl(MODCORE_DMA_REG_CNTL);
		cntl &= ~MODCORE_DMA_START;
		writel(cntl, MODCORE_DMA_REG_CNTL);
		/*printk("dma done!\n");*/
	}

	cntl = readl(MODCORE_DMA_REG_CNTL);
	cntl |= MODCORE_DMA_IRQ_DONE;
	writel(cntl, MODCORE_DMA_REG_CNTL);

	return 0;
}
