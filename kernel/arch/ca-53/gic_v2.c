// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

/**
 * @addtogroup kernel
 * @{
 * @addtogroup kernel_arch Arch
 * @{
 * @addtogroup kernel_arch_ca53 Cortex-A53
 * @{
 *
 * @file
 * @brief General Interrupt Controller (GIC) version 2 implementation
 */

/** Compatible string in devicetree for GIC 400 */
#define DEVICE_DT_COMPAT ARM_GIC_400

#include <regops.h>
#include <gic_v2.h>
#include <timer.h>
#include <sgi.h>
#include <generated_devicetree_defs.h>

/** Define GIC device from devicetree */
DEVICE_DEFINE_IDX(gic, 0);

/** GIC instance */
static struct device *gic_dev = DEVICE_GET_IDX(gic, 0);
/** Interrupt handler vector table for CPU 0 */
static struct ihandler handler[NUM_IRQS];

int32_t init_gic_dist(const struct device *dev)
{
	uint32_t ext_irq_num = 0;

	/* Disabling GIC */
	write32(dev->base_addr + GICD_CTLR_OFFSET, 0);

	/* GICD_TYPER: interrupt controller type reg.
	 * Find out how many interrupts are supported.
	 */
	ext_irq_num = read32(dev->base_addr + GICD_TYPER_OFFSET) & 0x1f;
	ext_irq_num = (ext_irq_num + 1) * 32;

	/* PPI interrupt sensitivity are fixed and doesn't need to be set.
	 * CPU private timer intr (secure private timer #29, non-secure
	 * private timer #30) can't be edited. All others are don't-care.
	 */

	/* Banked register set-enable ICDISER0.
	 * writing 1 enables intr.
	 * writing 0 has no effect.
	 * Enable all PPIs including secure private timer interrupt #29
	 */
	write32(dev->base_addr + GICD_ISE_BASE_OFFSET, 0xFFFF0000);

	/* Enable GIC distributor */
	write32(dev->base_addr + GICD_CTLR_OFFSET, 0x1);

	return 0;
}

int32_t init_gic_cpu(struct device *dev)
{
	if (dev == NULL) {
		return -EINVAL;
	}

	/* 32 priority level (ICCPMR[2:0] = 2b000)
	 * supported. ICDIPR0 ~ 23 is used to set
	 * interrupt priority. Reset value 0.
	 * Priority mask for the lowest priority
	 * which has max value of priority.
	 * Pass all levels of interrupts.
	 */
	write32(dev->base_addr + GICC_PMR_OFFSET, 0xF8);

	/* enable GIC cpu interface,
	 * banked register
	 */
	write32(dev->base_addr + GICC_CTLR_OFFSET, 0x1);

	return 0;
}

int32_t init_gic(struct device *dev)
{
	if (dev == NULL) {
		return -EINVAL;
	}

	dev->base_addr = DT_GET_BASE_ADDR(0);

	init_gic_dist(dev);
	init_gic_cpu(dev);

	return 0;
}

void gic_fiq(void)
{ /* do nothing */
}

int32_t gic_irq_handler(void)
{
	uint32_t ret;
	uint32_t num;
	uint32_t val;

	/* ack the interrupt */
	val = read32(gic_dev->base_addr + GICC_IAR_OFFSET);

	num = val & 0x3FF;

	if (num >= NUM_IRQS) {
		return -1;
	}

	ret = handler[num].func(handler[num].arg);

	write32(gic_dev->base_addr + GICC_EOIR_OFFSET, val);

	return ret;
}

int32_t gic_enable_interrupt(const struct device *dev, int vec)
{
	uint32_t reg;
	uint32_t bit;
	uint32_t val;

	/* register set-enable ICDISER0~2, only GIC_ICDISER0 is banked */
	reg = dev->base_addr + GICD_ISE_BASE_OFFSET + (uint32_t)(vec / 32) * 4;
	bit = 1 << (vec & 0x1F);

	/* writing 1 enables intr */
	val = read32(reg);
	val |= bit;
	write32(reg, bit);

	return 0;
}

int32_t gic_disable_interrupt(const struct device *dev, int vec)
{
	uint32_t reg;
	uint32_t bit;

	/* banked register clear-enable ICDICER0 */
	reg = dev->base_addr + GICD_ICE_BASE_OFFSET + (uint32_t)(vec / 32) * 4;
	bit = 1 << (vec & 31);

	/* writing 1 disables intr
	 * writing 0 has no effect
	 */
	write32(reg, bit);

	return 0;
}

int32_t gic_register_int_handler(int vec, int_handler func, void *arg)
{
	handler[vec].func = func;
	handler[vec].arg = arg;

	return 0;
}

int register_irq(const struct device *dev, int32_t (*irq_handler)(void *))
{
	(void)gic_register_int_handler(dev->irq, irq_handler, (void *)dev);

	return gic_enable_interrupt(gic_dev, dev->irq);
}

/**
 * @}
 * @}
 * @}
 */
 