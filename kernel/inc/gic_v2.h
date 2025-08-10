/* SPDX-License-Identifier: MIT OR Apache-2.0
 *
 * Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>
 */

/**
 * @addtogroup kernel
 * @{
 * @addtogroup kernel_arch Arch
 * @{
 * @addtogroup kernel_arch_ca53 Cortex-A53
 * @{
 *
 * @brief General Interrupt Controller (GIC) version 2 implementation
 */

#ifndef _GIC_V2_H_
#define _GIC_V2_H_

#include <stdint.h>

#include <device.h>

DEVICE_DECLARE_IDX(gic, 0);

/* GIC Distributor register offset */
#define GICD_CTLR_OFFSET       (0x00010000U)
#define GICD_TYPER_OFFSET      (0x00010004U)
#define GICD_IIDR_OFFSET       (0x00010008U)
#define GICD_GRP_BASE_OFFSET   (0x00010080U)
#define GICD_ISE_BASE_OFFSET   (0x00010100U)
#define GICD_ICE_BASE_OFFSET   (0x00010180U)
#define GICD_ISP_BASE_OFFSET   (0x00010200U)
#define GICD_ICP_BASE_OFFSET   (0x00010280U)
#define GICD_ISA_BASE_OFFSET   (0x00010300U)
#define GICD_ICA_BASE_OFFSET   (0x00010380U)
#define GICD_IPRIO_BASE_OFFSET (0x00010400U)
#define GICD_ITGT_BASE_OFFSET  (0x00010800U)
#define GICD_ICFG_BASE_OFFSET  (0x00010C00U)

/* GIC CPU Interface register offset */
#define GICC_CTLR_OFFSET (0x00020000U)
#define GICC_PMR_OFFSET	 (0x00020004U)
#define GICC_BPR_OFFSET	 (0x00020008U)
#define GICC_IAR_OFFSET	 (0x0002000CU)
#define GICC_EOIR_OFFSET (0x00020010U)

#define NUM_SGI	 16
#define NUM_PPI	 16
#define NUM_IRQS 187

typedef int (*int_handler)(void *arg);
struct ihandler {
	int_handler func;
	void *arg;
};

/**
 * @brief Initialize GIC disctributor
 *
 * @param dev GIC device controller instance
 * @return int32_t 0 for success, negative error code for failure
 */
int32_t init_gic_dist(const struct device *dev);

/**
 * @brief Initialize GIC CPU interface
 *
 * @param dev GIC device controller instance
 * @return int32_t 0 for success, negative error code for failure
 */
int32_t init_gic_cpu(struct device *dev);

/**
 * @brief Initialize GIC
 *
 * @param dev GIC device controller instance
 * @return int32_t 0 for success, negative error code for failure
 */
int32_t init_gic(struct device *dev);

/**
 * @brief IRQ handler called from IRQ exception
 *
 * @return int32_t 0 for success
 */
int32_t gic_irq_handler(void);

/**
 * @brief Enable interrupt
 *
 * @param [in] dev GIC device controller instance
 * @param [in] vec Interrupt number
 * @return int32_t 0 for success
 */
int32_t gic_enable_interrupt(const struct device *dev, int vec);

/**
 * @brief Disable interrupt
 *
 * @param [in] dev GIC device controller instance
 * @param [in] vec Interrupt number disabled
 * @return int32_t 0 for success
 */
int32_t gic_disable_interrupt(const struct device *dev, int vec);

/**
 * @brief Register interrupt handler to interrupt vector table
 *
 * @param [in] vec Interrupt number
 * @param [in] func Interrupt handler
 * @param [in] arg Interrupt handler argument
 * @return int32_t 0 for success
 */
int32_t gic_register_int_handler(int vec, int_handler func, void *arg);

/**
 * @brief Register IRQ handler and enable IRQ
 *
 * @param dev GIC device controller instance
 * @param irq_handler IRQ handler function
 * @return int32_t 0 for success
 */
int register_irq(const struct device *dev, int32_t (*irq_handler)(void *));
#endif

/**
 * @}
 * @}
 * @}
 *
 */

