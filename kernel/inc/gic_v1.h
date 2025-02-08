/* SPDX-License-Identifier: MIT OR Apache-2.0
 *
 * Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>
 */

/**
 * @addtogroup kernel
 * @{
 * @addtogroup kernel_arch Arch
 * @{
 * @addtogroup kernel_arch_ca9 Cortex-A9
 * @{
 *
 * @file
 * @brief General Interrupt Controller (GIC) version 1 implementation
 */

#ifndef _GIC_V1_H_
#define _GIC_V1_H_

#include <stdint.h>

/** GIC CPU interface reigsers */
#define GIC_ICCICR_OFFSET  0x100
#define GIC_ICCPMR_OFFSET  0x104
#define GIC_ICCIAR_OFFSET  0x10C
#define GIC_ICCEOIR_OFFSET 0x110
/** GIC Distributor registers */
#define GIC_ICDDCR_OFFSET   0x1000
#define GIC_ICDICTR_OFFSET  0x1004
#define GIC_ICDICFR0_OFFSET 0x1C00
#define GIC_ICDICFR1_OFFSET 0x1C04
#define GIC_ICDICFR2_OFFSET 0x1C08
#define GIC_ICDICFR3_OFFSET 0x1C0C
#define GIC_ICDICFR4_OFFSET 0x1C10
#define GIC_ICDICFR5_OFFSET 0x1C14
#define GIC_ICDIPTR0_OFFSET 0x1800
#define GIC_ICDIPTR8_OFFSET 0x1820
#define GIC_ICDISER0_OFFSET 0x1100
#define GIC_ICDISER1_OFFSET 0x1104
#define GIC_ICDISER2_OFFSET 0x1108
#define GIC_ICDICER0_OFFSET 0x1180
#define GIC_ICDICER1_OFFSET 0x1184
#define GIC_ICDICER2_OFFSET 0x1188

/** Software Generated Interrupt (SGI) number */
#define NUM_SGI 16
/** Private Peripheral Interrupt (PPI) number */
#define NUM_PPI 16
/** Shared Peripheral Interrupt (SPI) start number */
#define SPI_BASE (NUM_PPI + NUM_SGI)
/** Shared Peripheral Interrupt (SPI) number */
#define NUM_SPI 64
/** Total IRQ number */
#define NUM_IRQS (NUM_SGI + NUM_PPI + NUM_SPI)

/** Interrupt handler typedef */
typedef int32_t (*int_handler)(void *arg);

/** Interrupt vector table entry type */
struct ihandler {
	int_handler func;
	void *arg;
};

/**
 * @brief Initialize GIC
 */
void init_gic(void);

/**
 * @brief Initialize GIC Distributor
 *
 */
void init_gic_dist(void);

/**
 * @brief Initialize GIC CPU Interface
 *
 */
void init_gic_cpu(void);

/**
 * @brief Initialize GIC for secondary CPU (CPU 1)
 */
void init_gic_secondary(void);

/**
 * @brief Enable a specific interrupt
 *
 * @param [in] vec Interrupt number
 * @return uint32_t 0 for success
 */
uint32_t gic_enable_interrupt(int vec);

/**
 * @brief Disable a specific interrupt
 *
 * @param [in] vec Interrupt number
 * @return uint32_t 0 for success
 */
uint32_t gic_disable_interrupt(int vec);

/**
 * @brief Register interrupt handler to interrupt vector table
 *
 * @param [in] vec Interrupt number
 * @param [in] func Interrupt handler function
 * @param [in] arg Interrupt handler argument
 */
void gic_register_int_handler(int vec, int_handler func, void *arg);

/**
 * @brief GIC IRQ handler
 *
 * This is main IRQ handler called from IRQ exception handler
 *
 * @return uint32_t  Return IRQ handler return value
 */
uint32_t gic_irq_handler(void);

#endif

/**
 * @}
 * @}
 * @}
 *
 */
