/*
  kernel/inc/dma.h dma coprocessor driver header
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

#include <stdint-gcc.h>

#define DMA_REG_CNTL		0x43c00000
#define DMA_REG_STATUS		0x43c00004
#define DMA_REG_SRC_ADDR	0x43c00008
#define DMA_REG_LEN		0x43c0000c
#define DMA_REG_DST_ADDR	0x43c00010

#define DMA_IRQ_ID		61U

#define DMA_START		0x1
#define DMA_IRQ_DONE		0x2

extern void flush_ent_cache(void);

struct dma_work_order {
	uint32_t order_num;
	uint32_t src;
	uint32_t dst;
	uint32_t len;
	struct dma_work_order *next;
};

void init_dma(void);
void set_dma_work(uint32_t src, uint32_t dst, uint32_t len);
int start_dma(void);
int dma_irq (void *arg);
