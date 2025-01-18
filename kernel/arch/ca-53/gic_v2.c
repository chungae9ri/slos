// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

#define DEVICE_DT_COMPAT ARM_GIC_400

#include <regops.h>
#include <gic_v2.h>
#include <timer.h>
#include <sgi.h>
#include <generated_devicetree_defs.h>
#include <device.h>

DEVICE_DEFINE(gic_0, DT_GET_COMPAT(0), DT_GET_BASE_ADDR(0), 0);

static struct ihandler handler[NUM_IRQS];

void init_gic_dist(void)
{
	uint32_t ext_irq_num = 0;

	/* Disabling GIC */
	write32(DEVICE_GET_BASE_ADDR(gic_0) + GICD_CTLR_OFFSET, 0);

	/* GICD_TYPER: interrupt controller type reg.
	 * Find out how many interrupts are supported.
	 */
	ext_irq_num = read32(DEVICE_GET_BASE_ADDR(gic_0) + GICD_TYPER_OFFSET) & 0x1f;
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
	write32(DEVICE_GET_BASE_ADDR(gic_0) + GICD_ISE_BASE_OFFSET, 0xFFFF0000);

	/* Enable GIC distributor */
	write32(DEVICE_GET_BASE_ADDR(gic_0) + GICD_CTLR_OFFSET, 0x1);
}

void init_gic_cpu(void)
{
	/* 32 priority level (ICCPMR[2:0] = 2b000)
	 * supported. ICDIPR0 ~ 23 is used to set
	 * interrupt priority. Reset value 0.
	 * Priority mask for the lowest priority
	 * which has max value of priority.
	 * Pass all levels of interrupts.
	 */
	write32(DEVICE_GET_BASE_ADDR(gic_0) + GICC_PMR_OFFSET, 0xF8);

	/* enable GIC cpu interface,
	 * banked register
	 */
	write32(DEVICE_GET_BASE_ADDR(gic_0) + GICC_CTLR_OFFSET, 0x1);
}

void init_gic(void)
{
	init_gic_dist();
	init_gic_cpu();
}

void gic_fiq(void)
{ /* do nothing */
}

uint32_t gic_irq_handler(void)
{
	uint32_t ret;
	uint32_t num;
	uint32_t val;
	struct sgi_data dat = {0, 0};

	/* ack the interrupt */
	val = read32(DEVICE_GET_BASE_ADDR(gic_0) + GICC_IAR_OFFSET);

	num = val & 0x3FF;

	if (num >= NUM_IRQS) {
		return 1;
	}

	ret = handler[num].func(&dat);
	/* clear timer int(29U) status bit */
	if (num == PRIV_TMR_INT_VEC) {
		write32(PRIV_TMR_INTSTAT, 1);
	}

	write32(DEVICE_GET_BASE_ADDR(gic_0) + GICC_EOIR_OFFSET, val);

	return ret;
}

uint32_t gic_enable_interrupt(int vec)
{
	uint32_t reg;
	uint32_t bit;
	uint32_t val;

	/* register set-enable ICDISER0~2, only GIC_ICDISER0 is banked */
	reg = DEVICE_GET_BASE_ADDR(gic_0) + GICD_ISE_BASE_OFFSET + (uint32_t)(vec / 32) * 4;
	bit = 1 << (vec & 0x1F);

	/* writing 1 enables intr */
	val = read32(reg);
	val |= bit;
	write32(reg, bit);

	return 0;
}

uint32_t gic_disable_interrupt(int vec)
{
	uint32_t reg;
	uint32_t bit;

	/* banked register clear-enable ICDICER0 */
	reg = DEVICE_GET_BASE_ADDR(gic_0) + GICD_ICE_BASE_OFFSET + (uint32_t)(vec / 32) * 4;
	bit = 1 << (vec & 31);

	/* writing 1 disables intr
	 * writing 0 has no effect
	 */
	write32(reg, bit);

	return 0;
}

void gic_register_int_handler(int vec, int_handler func, void *arg)
{
	handler[vec].func = func;
	handler[vec].arg = arg;
}
