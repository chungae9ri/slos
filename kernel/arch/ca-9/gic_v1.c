// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

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

/** Devicetree compatible string */
#define DEVICE_DT_COMPAT ARM_GIC_390

#include <errno.h>
#include <regops.h>
#include <gic_v1.h>
#include <timer.h>
#include <sgi.h>
#include <percpudef.h>
#include <generated_devicetree_defs.h>
#include <device.h>
#include <ops.h>

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

/** Define GIC device from devicetree */
DEVICE_DEFINE_IDX(gic, 0);

/** GIC instance */
static struct device *gic_dev = DEVICE_GET_IDX(gic, 0);

/** Interrupt handler vector table for CPU 0 and CPU 1 */
static struct ihandler handler[NR_CPUS][NUM_IRQS];

/**
 * @brief Initialize GIC distributor
 *
 * @param dev GIC device controller instance
 */
static int32_t init_gic_dist(const struct device *dev)
{
	uint32_t i;
	uint32_t ext_irq_num = 0;
	/* 0x01010101 : CPU0 targeted */
	uint32_t cpumask = 0x01010101;

	if (dev == NULL) {
		return -EINVAL;
	}

	/* Disabling GIC */
	write32(dev->base_addr + GIC_ICDDCR_OFFSET, 0);

	/* GIC_ICDICTR: interrupt controller type reg.
	 * Find out how many interrupts are supported.
	 */
	ext_irq_num = read32(dev->base_addr + GIC_ICDICTR_OFFSET) & 0x1f;
	ext_irq_num = (ext_irq_num + 1) * 32;

	/* Set each interrupt line based on UG585
	 * interrupt polarities are set
	 * GIC_ICDICFR0~1 are ro
	 * Only 3 intrs are configured
	 * 1. CPU private timer intr #29 can't be edited.
	 * 2. GIC_ICDICFR3[27:26] is 01 for high level
	 * polarity for intr ID #61 of dma intr
	 * 3. GIC_ICDICFR3[29:28] is 01 for high level
	 * polarity for intr ID #62 of odev intr
	 * All others are don't-care
	 */
	write32(dev->base_addr + GIC_ICDICFR2_OFFSET, 0x555D555F);
	write32(dev->base_addr + GIC_ICDICFR3_OFFSET, 0xDD55D555);
	write32(dev->base_addr + GIC_ICDICFR4_OFFSET, 0x755557FF);
	write32(dev->base_addr + GIC_ICDICFR5_OFFSET, 0x03FFFF55);

	/* setup target cpu for each interrupt
	 * all intr are targeted to CPU0.
	 * GIC_ICDIPTR0~7 for intr #0~#31 are ro.
	 * SPI will be retargeted when it is
	 * enabled in gic_enable_interrupt().
	 */
	for (i = 0; i < ext_irq_num; i += 4)
		write32(dev->base_addr + GIC_ICDIPTR8_OFFSET + i, cpumask);

	/* Disabling all interrupts forwarding */
	write32(dev->base_addr + GIC_ICDICER0_OFFSET, 0x0000FFFF);
	write32(dev->base_addr + GIC_ICDICER1_OFFSET, 0xFFFFFFFF);
	write32(dev->base_addr + GIC_ICDICER2_OFFSET, 0xFFFFFFFF);

	/* banked register set-enable ICDISER0.
	 * writing 1 enables intr.
	 * writing 0 has no effect.
	 * Enable all PPIs and SGI #15
	 */
	write32(dev->base_addr + GIC_ICDISER0_OFFSET, 0xFFFF8000);

	/* enable TTC0 interrupt forwarding, not here */
	/*write32(GIC_ICDISER1, 0x00001C00);*/

	/* enable GIC distributor to update intr register
	 * in both secure, non-secure interrupt singal occurrance
	 */
	write32(dev->base_addr + GIC_ICDDCR_OFFSET, 0x3);

	return 0;
}

static int32_t init_gic_cpu(const struct device *dev)
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
	write32(dev->base_addr + GIC_ICCPMR_OFFSET, 0xF8);
	/* enable GIC cpu interface,
	 * banked register
	 */
	write32(dev->base_addr + GIC_ICCICR_OFFSET, 0x07);

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

int32_t init_gic_secondary(const struct device *dev)
{
	if (dev == NULL) {
		return -EINVAL;
	}

	/* Disabling interrupt forwarding is already done
	 * in init_gic_dist(), which should be done before
	 * gic_enable_interrupt()
	 */

	/* enable GIC distributor,
	 * banked register
	 */
	write32(dev->base_addr + GIC_ICDDCR_OFFSET, 0x1);

	/* 32 priority level (ICCPMR[2:0] = 2b000)
	 * supported. ICDIPR0 ~ 23 is used to set
	 * interrupt priority. Reset value 0.
	 * Priority mask for the lowest priority
	 * which has max value of priority.
	 * Pass all levels of interrupts.
	 */
	write32(dev->base_addr + GIC_ICCPMR_OFFSET, 0xF8);

	/* enable GIC cpu interface,
	 * banked register.
	 */
	write32(dev->base_addr + GIC_ICCICR_OFFSET, 0x7);

	/* banked register set-enable ICDISER0.
	 * writing 1 enables intr.
	 * writing 0 has no effect.
	 * Enable all PPIs and SGI #15
	 * Each SPI enable is set by calling gic_enable_interrupt
	 */
	write32(dev->base_addr + GIC_ICDISER0_OFFSET, 0xFFFF8000);
	/*write32(GIC_ICDISER0, 0xFFFFFFFF);*/

	return 0;
}

void gic_fiq(void)
{ /* do nothing */
}

int32_t gic_irq_handler(void)
{
	int32_t ret;
	uint32_t num;
	uint32_t val;
	uint32_t cpuid;

	/* ack the interrupt */
	val = read32(gic_dev->base_addr + GIC_ICCIAR_OFFSET);

	/* get current cpuid */
	cpuid = smp_processor_id();

	num = val & 0x3FF;
	if (num >= NUM_IRQS) {
		return 1;
	}

	/* run ISR */
	ret = handler[cpuid][num].func(handler[cpuid][num].arg);

	/* end of interrupt */
	write32(gic_dev->base_addr + GIC_ICCEOIR_OFFSET, val);

	return ret;
}

int32_t gic_enable_interrupt(const struct device *dev, int vec)
{
	uint32_t reg;
	uint32_t bit;
	uint32_t val;
	uint32_t byte;
	uint32_t cpuid;

	if (dev == NULL || vec < 0 || vec >= NUM_IRQS) {
		return -EINVAL;
	}

	/* only SPI can be set with the target CPU */
	if (vec >= SPI_BASE) {
		/* get current cpuid */
		cpuid = smp_processor_id();
		/* reprogram GIC_ICDIPTR */
		reg = dev->base_addr + GIC_ICDIPTR0_OFFSET + (uint32_t)(vec / 4) * 4;
		byte = (uint32_t)(vec % 4);
		val = read32(reg);
		/* clear current cpuid in the byte */
		val = val & ~(0x000000FF << (byte * 8));
		/* set the cpuid in the byte */
		val = val | ((0x1 << cpuid) << (byte * 8));
		write32(reg, val);
	}

	/* register set-enable ICDISER0~2, only GIC_ICDISER0 is banked */
	reg = dev->base_addr + GIC_ICDISER0_OFFSET + (uint32_t)(vec / 32) * 4;
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

	if (dev == NULL || vec < 0 || vec >= NUM_IRQS) {
		return -EINVAL;
	}

	/* banked register clear-enable ICDICER0 */
	reg = dev->base_addr + GIC_ICDICER0_OFFSET + (uint32_t)(vec / 32) * 4;
	bit = 1 << (vec & 31);

	/* writing 1 disables intr
	 * writing 0 has no effect
	 */
	write32(reg, bit);

	return 0;
}

int32_t gic_register_int_handler(int vec, int_handler func, void *arg)
{
	uint32_t cpuid;

	/* get current cpuid to register isr for
	 * the corresponding cpu
	 */
	cpuid = smp_processor_id();

	handler[cpuid][vec].func = func;
	handler[cpuid][vec].arg = arg;

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

