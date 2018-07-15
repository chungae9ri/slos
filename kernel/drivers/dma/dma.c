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

#include <dma.h>
#include <regops.h>
#include <gic.h>
#include <mm.h>
#include <xil_printf.h>

#define DMA_BURST_LEN	0x1000
#define TEST_DMA	0

#if TEST_DMA
struct dma_work_order dma_work_order;
#else
struct dma_work_order *p_dma_work_order;
#endif


void init_dma(void)
{
	gic_register_int_handler(DMA_IRQ_ID, dma_irq, NULL);
	gic_mask_interrupt(DMA_IRQ_ID);
#if TEST_DMA
#else
	p_dma_work_order = NULL;
#endif
}

void set_dma_work(uint32_t src, uint32_t dst, uint32_t len)
{
#if TEST_DMA
	dma_work_order.src = src;
	dma_work_order.dst = dst;
	dma_work_order.len = len;
#else
	int i, q, r;
	struct dma_work_order *pcur, *ptemp;

	if (len > DMA_BURST_LEN) {
		q = (int)(len / DMA_BURST_LEN);
		r = len % DMA_BURST_LEN;

		for (i = 0; i < q; i++) {
			ptemp = (struct dma_work_order *)kmalloc(sizeof(struct dma_work_order));
			ptemp->order_num = i;
			ptemp->src = src + DMA_BURST_LEN * i;
			ptemp->dst = dst + DMA_BURST_LEN * i;
			ptemp->len = DMA_BURST_LEN;
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
			ptemp->src = src + DMA_BURST_LEN * i;
			ptemp->dst = dst + DMA_BURST_LEN * i;
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
		ptemp->len = DMA_BURST_LEN;
		ptemp->next = NULL;

		p_dma_work_order = ptemp;
	}
#endif
}

int start_dma(void)
{
#if TEST_DMA
	uint32_t cntl, src, dst, len;

	src = dma_work_order.src;
	dst = dma_work_order.dst;
	len = dma_work_order.len;

	writel(src, DMA_REG_SRC_ADDR);
	writel(dst, DMA_REG_DST_ADDR);
	writel(len, DMA_REG_LEN);

	cntl = readl(DMA_REG_CNTL);
	cntl |= DMA_START;
	writel(cntl, DMA_REG_CNTL);

	return 0;

#else
	uint32_t cntl, src, dst, len;
	struct dma_work_order *ptemp;

	if (p_dma_work_order) {
		src = p_dma_work_order->src;
		dst = p_dma_work_order->dst;
		len = p_dma_work_order->len;

		writel(src, DMA_REG_SRC_ADDR);
		writel(dst, DMA_REG_DST_ADDR);
		writel(len, DMA_REG_LEN);

		ptemp = p_dma_work_order;
		p_dma_work_order = p_dma_work_order->next;
		kfree((uint32_t)ptemp);

		cntl = readl(DMA_REG_CNTL);
		cntl |= DMA_START;
		writel(cntl, DMA_REG_CNTL);

		return 0;
	} else {
		return 1;
	}
#endif
}

int dma_irq (void *arg)
{
#if TEST_DMA
	uint32_t cntl;

	cntl = readl(DMA_REG_CNTL);
	cntl |= DMA_IRQ_DONE;
	cntl &= ~DMA_START;
	writel(cntl, DMA_REG_CNTL);

	xil_printf("dma done!\n");
#else
	uint32_t cntl;

	cntl = readl(DMA_REG_CNTL);
	cntl |= DMA_IRQ_DONE;
	writel(cntl, DMA_REG_CNTL);

	/* start next dma order */
	if (start_dma()) {
		cntl = readl(DMA_REG_CNTL);
		cntl &= ~DMA_START;
		writel(cntl, DMA_REG_CNTL);

		xil_printf("dma done!\n");
	} 

	return 0;
#endif
}
