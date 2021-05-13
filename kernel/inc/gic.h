/*
  kernel/inc/gic.h 
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

#define GIC_ICCICR	XPS_SCU_PERIPH_BASE + 0x100
#define GIC_ICCPMR	XPS_SCU_PERIPH_BASE + 0x104
#define GIC_ICCBPR	XPS_SCU_PERIPH_BASE + 0x108
#define GIC_ICCIAR	XPS_SCU_PERIPH_BASE + 0x10C
#define GIC_ICCEOIR	XPS_SCU_PERIPH_BASE + 0x110
#define GIC_ICCRPR	XPS_SCU_PERIPH_BASE + 0x114
#define GIC_ICCHPIR	XPS_SCU_PERIPH_BASE + 0x118
#define GIC_ICCABPR	XPS_SCU_PERIPH_BASE + 0x11C
#define GIC_ICCIDR	XPS_SCU_PERIPH_BASE + 0x120
#define GIC_ICDDCR	XPS_SCU_PERIPH_BASE + 0x1000
#define GIC_ICDICTR	XPS_SCU_PERIPH_BASE + 0x1004
#define GIC_ICDIIDR	XPS_SCU_PERIPH_BASE + 0x1008
#define GIC_ICDICFR0	XPS_SCU_PERIPH_BASE + 0x1C00
#define GIC_ICDICFR1	XPS_SCU_PERIPH_BASE + 0x1C04
#define GIC_ICDICFR2	XPS_SCU_PERIPH_BASE + 0x1C08
#define GIC_ICDICFR3	XPS_SCU_PERIPH_BASE + 0x1C0C
#define GIC_ICDICFR4	XPS_SCU_PERIPH_BASE + 0x1C10
#define GIC_ICDICFR5	XPS_SCU_PERIPH_BASE + 0x1C14
#define GIC_ICDIPR0	XPS_SCU_PERIPH_BASE + 0x1400
#define GIC_ICDIPR1	XPS_SCU_PERIPH_BASE + 0x1404
#define GIC_ICDIPR2	XPS_SCU_PERIPH_BASE + 0x1408
#define GIC_ICDIPR3	XPS_SCU_PERIPH_BASE + 0x140C
#define GIC_ICDIPR4	XPS_SCU_PERIPH_BASE + 0x1410
#define GIC_ICDIPR5	XPS_SCU_PERIPH_BASE + 0x1414
#define GIC_ICDIPR6	XPS_SCU_PERIPH_BASE + 0x1418
#define GIC_ICDIPR7	XPS_SCU_PERIPH_BASE + 0x141C
#define GIC_ICDIPR8	XPS_SCU_PERIPH_BASE + 0x1420
#define GIC_ICDIPR9	XPS_SCU_PERIPH_BASE + 0x1424
#define GIC_ICDIPTR0	XPS_SCU_PERIPH_BASE + 0x1800
#define GIC_ICDIPTR8	XPS_SCU_PERIPH_BASE + 0x1820
#define GIC_ICDIPTR23	XPS_SCU_PERIPH_BASE + 0x185C
#define GIC_ICDISER0	XPS_SCU_PERIPH_BASE + 0x1100
#define GIC_ICDISER1	XPS_SCU_PERIPH_BASE + 0x1104
#define GIC_ICDISER2	XPS_SCU_PERIPH_BASE + 0x1108
#define GIC_ICDICER0	XPS_SCU_PERIPH_BASE + 0x1180
#define GIC_ICDICER1	XPS_SCU_PERIPH_BASE + 0x1184
#define GIC_ICDICER2	XPS_SCU_PERIPH_BASE + 0x1188
#define GIC_PPISTATUS	XPS_SCU_PERIPH_BASE + 0x1D00
#define GIC_SPISTATUS0	XPS_SCU_PERIPH_BASE + 0x1D04
#define GIC_SPISTATUS1	XPS_SCU_PERIPH_BASE + 0x1D08

#define NUM_SGI			16
#define NUM_PPI			16
#define NUM_SPI			64
#define NUM_IRQS	(NUM_SGI + NUM_PPI + NUM_SPI)

#define SPI_BASE	
#define TTC0_TIMER0_INT		42
#define TTC0_TIMER1_INT		43
#define TTC0_TIMER2_INT		44

#define TTC1_TIMER0_INT		69
#define TTC1_TIMER1_INT		70
#define TTC1_TIMER2_INT		71

typedef int (*int_handler)(void *arg);
struct ihandler {
	int_handler func;
	void *arg;
};

struct ihandler handler[NUM_IRQS];

void init_gic(void);
void init_gic_secondary(void);
uint32_t gic_mask_interrupt(int vec);
uint32_t gic_unmask_interrupt(int vec);
void gic_register_int_handler(int vec, int_handler func, void *arg);
