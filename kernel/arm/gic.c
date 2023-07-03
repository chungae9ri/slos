/*
  kernel/arm/gic.c general interrupt controller 
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

#include <xparameters_ps.h>
#include <regops.h>
#include <gic.h>
#include <timer.h>
#include <sgi.h>

uint32_t smp_processor_id();
struct ihandler handler[NUM_IRQS];

void init_gic_dist(void)
{
	uint32_t i;
	uint32_t ext_irq_num = 0;
	/* 0x01010101 : CPU0 targeted */
	uint32_t cpumask = 0x01010101;

	/* Disabling GIC */
	writel(0, GIC_ICDDCR);

	/*  
	 * GIC_ICDICTR: interrupt controller type reg.
	 * Find out how many interrupts are supported.
	 */
	ext_irq_num = readl(GIC_ICDICTR) & 0x1f;
	ext_irq_num = ext_irq_num * 32; 

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
	writel(0x555D555F, GIC_ICDICFR2);
	writel(0xDD55D555, GIC_ICDICFR3);
	writel(0x755557FF, GIC_ICDICFR4);
	writel(0x03FFFF55, GIC_ICDICFR5);

	/* setup target cpu for each interrupt 
	 * all intr are targeted to CPU0.
	 * GIC_ICDIPTR0~7 for intr #0~#31 are ro.
	 * SPI will be retargeted when it is 
	 * enabled in gic_mask_interrupt().
	 */
	for (i = 0; i < ext_irq_num; i += 4)
		writel(cpumask, GIC_ICDIPTR8 + i); 

	/* Disabling all interrupts forwarding */
	writel(0x0000FFFF, GIC_ICDICER0);
	writel(0xFFFFFFFF, GIC_ICDICER1);
	writel(0xFFFFFFFF, GIC_ICDICER2);

	/* banked register set-enable ICDISER0. 
	 * writing 1 enables intr.
	 * writing 0 has no effect. 
	 * Enable all PPIs and SGI #15
	 */
	writel(0xFFFF8000, GIC_ICDISER0);

	/* enable TTC0 interrupt forwarding, not here */
	/*writel(0x00001C00, GIC_ICDISER1);*/

	/* enable GIC distributor to update intr register 
	 * in both secure, non-secure interrupt singal occurrance
	 */
	writel(0x3, GIC_ICDDCR);
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
	writel(0xF8, GIC_ICCPMR);
	/* enable GIC cpu interface,
	 * banked register
	 */ 
	writel(0x07, GIC_ICCICR);
}

void init_gic(void)
{
	init_gic_dist();
	init_gic_cpu();
}

void init_gic_secondary(void)
{
	/* Disabling interrupt forwarding is already done
	 * in init_gic_dist(), which should be done before
	 * gic_mask_interrupt()
	 */
#if 0
	/* Disabling interrupts forwarding */
	writel(0x0000FFFF, GIC_ICDICER0);
	writel(0xFFFFFFFF, GIC_ICDICER1);
	writel(0xFFFFFFFF, GIC_ICDICER2);
#endif

	/* enable GIC distributor, 
	 * banked register 
	 */
	writel(0x1, GIC_ICDDCR);

	/* 32 priority level (ICCPMR[2:0] = 2b000)
	 * supported. ICDIPR0 ~ 23 is used to set 
	 * interrupt priority. Reset value 0.
	 * Priority mask for the lowest priority
	 * which has max value of priority.
	 * Pass all levels of interrupts.
	 */
	writel(0xF8, GIC_ICCPMR);

	/* enable GIC cpu interface, 
	 * banked register.
	 */ 
	writel(0x7, GIC_ICCICR);

	/* banked register set-enable ICDISER0. 
	 * writing 1 enables intr.
	 * writing 0 has no effect. 
	 * Enable all PPIs and SGI #15
	 * Each SPI enable is set by calling gic_mask_interrupt
	 */
	writel(0xFFFF8000, GIC_ICDISER0);
	/*writel(0xFFFFFFFF, GIC_ICDISER0);*/
}

void gic_fiq(void)
{
	/* do nothing */
}

/*imsi for test uint32_t gic_irq(struct task_struct *frame)*/
uint32_t gic_irq_handler(void)
{
	uint32_t ret = 0;
	uint32_t num, val;
	struct sgi_data dat = {0, 0};

	/* ack the interrupt */
	val = readl(GIC_ICCIAR);

	num = val & 0x3FF;

	if (num >= NUM_IRQS) {
		return 1;
	} else if (num < NUM_SGI) {
		dat.cpuid = val & 0x1C00;
		dat.num = num;
	}

	ret = handler[num].func(&dat);
	/* clear timer int(29U) status bit */
	if (num == PRIV_TMR_INT_VEC) {
		writel(1, PRIV_TMR_INTSTAT);
	}

	writel(val, GIC_ICCEOIR);
	return ret;
}

uint32_t gic_mask_interrupt(int vec)
{
	uint32_t cpuid, reg, bit, val, byte;

	/* only SPI can be set with the target CPU */
	if (vec >= SPI_BASE) {
		/* get current cpuid */
		cpuid = smp_processor_id();
		/* reprogram GIC_ICDIPTR */
		reg = GIC_ICDIPTR0 + (uint32_t)(vec / 4) * 4;
		byte = (uint32_t)(vec % 4);
		val = readl(reg);
		/* clear current cpuid in the byte */
		val = val & (0xFFFFFFFF & (0x00 << (byte * 8)));
		/* set the cpuid in the byte */
		val = val | ((0x1 << cpuid) << (byte * 8));
		writel(val, reg);
	}

	/* register set-enable ICDISER0~2, only GIC_ICDISER0 is banked */
	reg = GIC_ICDISER0 + (uint32_t)(vec / 32) * 4;
	bit = 1 << (vec & 0x1F);

	/* writing 1 enables intr */
	val = readl(reg);
	val |= bit;
	writel(bit, reg);
	return 0;
}

uint32_t gic_unmask_interrupt(int vec)
{
	uint32_t reg;
	uint32_t bit;

	/* banked register clear-enable ICDICER0 */
	reg = GIC_ICDICER0 + (uint32_t)(vec / 32) * 4;
	bit = 1 << (vec & 31);

	/* 
	 * writing 1 disables intr
	 * writing 0 has no effect 
	 */
	writel(bit, reg);
	return 0;
}

void gic_register_int_handler(int vec, int_handler func, void *arg)
{
	handler[vec].func = func;
	handler[vec].arg = arg;
}


