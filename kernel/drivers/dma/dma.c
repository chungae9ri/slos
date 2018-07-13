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
#include <xil_printf.h>

void init_dma(void)
{
	gic_register_int_handler(DMA_IRQ_ID, dma_irq, NULL);
	gic_mask_interrupt(DMA_IRQ_ID);
}

void set_dma_work(uint32_t src, uint32_t dst, uint32_t len)
{
	writel(src, DMA_REG_SRC_ADDR);
	writel(dst, DMA_REG_DST_ADDR);
	writel(len, DMA_REG_LEN);
}

void start_dma(void)
{
	uint32_t cntl;

	cntl = readl(DMA_REG_CNTL);
	cntl |= DMA_START;
	writel(0x1, DMA_REG_CNTL);
}

int b = 0;
int dma_irq (void *arg)
{
	uint32_t cntl;
	cntl = readl(DMA_REG_CNTL);
	cntl |= DMA_IRQ_DONE;
	writel(cntl, DMA_REG_CNTL);
	xil_printf("dma irq done ! \n");

	/* start next dma */
	if (b == 0) {
		b = 1;
		set_dma_work(0x20002000, 0x20003000, 0x100);
		start_dma();
	}
	return 0;
}
