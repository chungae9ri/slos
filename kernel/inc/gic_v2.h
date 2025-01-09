// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

#ifndef _GIC_V2_H_
#define _GIC_V2_H_

#include <stdint.h>

/* GIC Distributor register offset */
#define GICD_CTLR_OFFSET			(0x00010000U)
#define GICD_TYPER_OFFSET			(0x00010004U)
#define GICD_IIDR_OFFSET			(0x00010008U)
#define GICD_GRP_BASE_OFFSET		(0x00010080U)
#define GICD_ISE_BASE_OFFSET		(0x00010100U)
#define GICD_ICE_BASE_OFFSET		(0x00010180U)
#define GICD_ISP_BASE_OFFSET		(0x00010200U)
#define GICD_ICP_BASE_OFFSET		(0x00010280U)
#define GICD_ISA_BASE_OFFSET		(0x00010300U)
#define GICD_ICA_BASE_OFFSET		(0x00010380U)
#define GICD_IPRIO_BASE_OFFSET		(0x00010400U)
#define GICD_ITGT_BASE_OFFSET		(0x00010800U)
#define GICD_ICFG_BASE_OFFSET		(0x00010C00U)

/* GIC CPU Interface register offset */
#define GICC_CTLR_OFFSET			(0x00020000U)
#define GICC_PMR_OFFSET				(0x00020004U)
#define GICC_BPR_OFFSET				(0x00020008U)
#define GICC_IAR_OFFSET				(0x0002000CU)
#define GICC_EOIR_OFFSET			(0x00020010U)

#define NUM_SGI             16
#define NUM_PPI             16
#define NUM_IRQS			187


typedef int (*int_handler)(void *arg);
struct ihandler {
	int_handler func;
	void *arg;
};

void init_gic(void);
void init_gic_secondary(void);
uint32_t gic_enable_interrupt(int vec);
uint32_t gic_disable_interrupt(int vec);
void gic_register_int_handler(int vec, int_handler func, void *arg);

#endif
