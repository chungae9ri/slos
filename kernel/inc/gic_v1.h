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
 * @brief General Interrupt Controller (GIC) version 1 implementation
 */

#ifndef _GIC_V1_H_
#define _GIC_V1_H_

#include <stdint.h>

#include <device.h>

DEVICE_DECLARE_IDX(gic, 0);

/** Interrupt handler typedef */
typedef int32_t (*int_handler)(void *arg);

/** Interrupt vector table entry type */
struct ihandler {
	int_handler func;
	void *arg;
};

/**
 * @brief Initialize GIC
 *
 * @param dev GIC device controller instance
 */
int32_t init_gic(struct device *dev);

/**
 * @brief Initialize GIC for secondary CPU (CPU 1)
 *
 * @param dev GIC device controller instance
 */
int32_t init_gic_secondary(const struct device *dev);

/**
 * @brief Enable a specific interrupt
 *
 * @param dev GIC device controller instance
 * @param [in] vec Interrupt number
 * @return int32_t 0 for success
 */
int32_t gic_enable_interrupt(const struct device *dev, int vec);

/**
 * @brief Disable a specific interrupt
 *
 * @param dev GIC device controller instance
 * @param [in] vec Interrupt number
 * @return int32_t 0 for success
 */
int32_t gic_disable_interrupt(const struct device *dev, int vec);

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
int32_t gic_irq_handler(void);

#endif

/**
 * @}
 * @}
 * @}
 *
 */

