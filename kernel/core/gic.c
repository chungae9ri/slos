/*
  kernel/core/gic.c general interrupt controller 
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

void init_gic_dist(void)
{
	uint32_t i;
	uint32_t num_irq = 0;
	uint32_t cpumask = 1;

	cpumask |= cpumask << 8;
	cpumask |= cpumask << 16; /* 0x01010101 : CPU0 targeted */

	/* Disabling GIC */
	writel(0, GIC_ICDDCR);

	/*  
	 * Find out how many interrupts are supported.
	 */
	num_irq = readl(GIC_ICDICTR) & 0x1f;
	num_irq = (num_irq + 1) * 32; 

	/* Set each interrupt line based on UG585 
	 * interrupt polarities are set
	 */
	writel(0x555D555F, GIC_ICDICFR2);
	writel(0xFD55D555, GIC_ICDICFR3);
	writel(0x755557FF, GIC_ICDICFR4);
	writel(0x03FFFF55, GIC_ICDICFR5);

	/* setup target cpu for each interrupt */
	for (i = 0; i < num_irq; i += 4)
		writel(cpumask, GIC_ICDIPTR8 + i); 

	/* Disabling interrupts forwarding */
	writel(0x0000FFFF, GIC_ICDICER0);
	writel(0xFFFFFFFF, GIC_ICDICER1);
	writel(0xFFFFFFFF, GIC_ICDICER2);

	/* Enable PPI interrupts forwarding */

	/* enable TTC0 interrupt forwarding, not here */
	/*writel(0x00001C00, GIC_ICDISER1);*/

	/*Enabling GIC */
	writel(1, GIC_ICDDCR);
}

void init_gic_cpu(void)
{
	writel(0xF0, GIC_ICCPMR);
	writel(0x07, GIC_ICCICR);
}

void init_gic(void)
{
	init_gic_dist();
	init_gic_cpu();
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

	/* ack the interrupt */
	val = readl(GIC_ICCIAR);
	/* cpuid is not used */
	/*cpuid = val & 0x1C00;*/
	num = val & 0x3FF;

	if (num >= NUM_IRQS) {
		return 1;
	}
	
	/*ret = handler[num].func(frame);*/
	ret = handler[num].func(0);
	/* clear timer int(29U) status bit */
	if (num == PRIV_TMR_INT_VEC) {
		writel(1, PRIV_TMR_INTSTAT);
	}

	writel(val, GIC_ICCEOIR);
	return ret;
}

uint32_t gic_mask_interrupt(int vec)
{
	uint32_t reg;
	uint32_t bit;

	reg = GIC_ICDISER0 + (uint32_t)(vec / 32) * 4;
	bit = 1 << (vec & 31);

	writel(bit, reg);
	return 0;
}

uint32_t gic_unmask_interrupt(int vec)
{
	uint32_t reg;
	uint32_t bit;

	reg = GIC_ICDICER0 + (uint32_t)(vec / 32) * 4;
	bit = 1 << (vec & 31);

	writel(bit, reg);
	return 0;
}

void gic_register_int_handler(int vec, int_handler func, void *arg)
{
	handler[vec].func = func;
	handler[vec].arg = arg;
}
