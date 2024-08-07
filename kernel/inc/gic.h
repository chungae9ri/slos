// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

#ifndef GIC_390_H
#define GIC_390_H

#include <stdint.h>

#define GIC_ICCICR_OFFSET 0x100
#define GIC_ICCPMR_OFFSET 0x104
#define GIC_ICCBPR_OFFSET 0x108
#define GIC_ICCIAR_OFFSET 0x10C
#define GIC_ICCEOIR_OFFSET 0x110
#define GIC_ICCRPR_OFFSET 0x114
#define GIC_ICCHPIR_OFFSET 0x118
#define GIC_ICCABPR_OFFSET 0x11C
#define GIC_ICCIDR_OFFSET 0x120
#define GIC_ICDDCR_OFFSET 0x1000
#define GIC_ICDICTR_OFFSET 0x1004
#define GIC_ICDIIDR_OFFSET 0x1008
#define GIC_ICDICFR0_OFFSET 0x1C00
#define GIC_ICDICFR1_OFFSET 0x1C04
#define GIC_ICDICFR2_OFFSET 0x1C08
#define GIC_ICDICFR3_OFFSET 0x1C0C
#define GIC_ICDICFR4_OFFSET 0x1C10
#define GIC_ICDICFR5_OFFSET 0x1C14
#define GIC_ICDIPR0_OFFSET 0x1400
#define GIC_ICDIPR1_OFFSET 0x1404
#define GIC_ICDIPR2_OFFSET 0x1408
#define GIC_ICDIPR3_OFFSET 0x140C
#define GIC_ICDIPR4_OFFSET 0x1410
#define GIC_ICDIPR5_OFFSET 0x1414
#define GIC_ICDIPR6_OFFSET 0x1418
#define GIC_ICDIPR7_OFFSET 0x141C
#define GIC_ICDIPR8_OFFSET 0x1420
#define GIC_ICDIPR9_OFFSET 0x1424
#define GIC_ICDIPTR0_OFFSET 0x1800
#define GIC_ICDIPTR8_OFFSET 0x1820
#define GIC_ICDIPTR23_OFFSET 0x185C
#define GIC_ICDISER0_OFFSET 0x1100
#define GIC_ICDISER1_OFFSET 0x1104
#define GIC_ICDISER2_OFFSET 0x1108
#define GIC_ICDICER0_OFFSET 0x1180
#define GIC_ICDICER1_OFFSET 0x1184
#define GIC_ICDICER2_OFFSET 0x1188
#define GIC_PPISTATUS_OFFSET 0x1D00
#define GIC_SPISTATUS0_OFFSET 0x1D04
#define GIC_SPISTATUS1_OFFSET 0x1D08

#define NUM_SGI             16
#define NUM_PPI             16
#define SPI_BASE            (NUM_PPI + NUM_SGI)	
#define NUM_SPI			    64
#define NUM_IRQS	        (NUM_SGI + NUM_PPI + NUM_SPI)

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


void init_gic(void);
void init_gic_secondary(void);
uint32_t gic_mask_interrupt(int vec);
uint32_t gic_unmask_interrupt(int vec);
void gic_register_int_handler(int vec, int_handler func, void *arg);

#endif
