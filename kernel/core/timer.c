/*
  SLOS timer
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

#include <stdlib.h>
#include <stdint-gcc.h>
#include <timer.h>
#include <gic.h>
#include <xparameters_ps.h>
#include <regops.h>
#include <xil_printf.h>
#include <rbtree.h>
#include <ktimer.h>
#include <defs.h>
#include <percpu.h>

extern uint32_t smp_processor_id();
static uint32_t ticks_per_sec;

static void delay(uint32_t ticks)
{
	volatile uint32_t cnt;
	uint32_t init_cnt;
	uint32_t timeout = 0;

	cnt = timer_get_phy_tick_cnt();
	init_cnt = cnt;

	if (init_cnt < ticks) {
		while (init_cnt >= cnt)
			cnt = timer_get_phy_tick_cnt();
		timeout = ticks - init_cnt;
		init_cnt = cnt;
		while (init_cnt - cnt <= timeout) 
			cnt = timer_get_phy_tick_cnt();
	} else {
		timeout = init_cnt - ticks;
		while (init_cnt - cnt <= timeout) 
			cnt = timer_get_phy_tick_cnt();
	}

}

void mdelay(unsigned msecs)
{
	uint64_t ticks;
	ticks = ((uint64_t)msecs * ticks_per_sec)/1000;
	delay(ticks);
}

void udelay(unsigned usecs)
{
	uint64_t ticks;
	ticks = ((uint64_t)usecs * ticks_per_sec)/1000000;
	delay(ticks);
}

inline uint32_t get_timer_freq()
{
	return (XPAR_CPU_CORTEXA9_0_CPU_CLK_FREQ_HZ / 2);
}

inline uint32_t timer_get_phy_tick_cnt(void)
{
	return (uint32_t)readl(PRIV_TMR_CNTR);
}

void timer_enable(void)
{
	int ctrl;

	ctrl = readl(PRIV_TMR_CTRL);
	ctrl = ctrl | (PRIV_TMR_EN_MASK 
			| PRIV_TMR_AUTO_RE_MASK
			| PRIV_TMR_IRQ_EN_MASK);
		
	writel(ctrl, PRIV_TMR_CTRL);
}

void timer_enable_secondary(void)
{
	uint32_t ctrl;
	/* init timer */
	*(volatile uint32_t *)(PRIV_TMR_LD) = 1000000;
	gic_mask_interrupt(PRIV_TMR_INT_VEC);
	*(volatile uint32_t *)(GIC_ICDICER0) = 0xDFFFFFFF;

	/* enable timer */
	ctrl = *(volatile uint32_t *)(PRIV_TMR_CTRL);
	ctrl = ctrl | (PRIV_TMR_EN_MASK 
			| PRIV_TMR_AUTO_RE_MASK
			| PRIV_TMR_IRQ_EN_MASK);

	*(volatile uint32_t *)(PRIV_TMR_CTRL) = ctrl;
}

void timer_disable(void)
{
	int ctrl;

	ctrl = readl(PRIV_TMR_CTRL);
	ctrl = ctrl & ~PRIV_TMR_EN_MASK;
		
	writel(ctrl, PRIV_TMR_CTRL);
}


int timer_irq (void *arg)
{
	uint32_t elapsed = 0;
	uint32_t tc = 0;
	struct timer_struct *pnt = NULL, *pct = NULL; 
	struct task_struct *this_current = NULL;
	struct timer_struct *this_sched_timer = NULL;
	struct timer_root *this_ptroot;
#if _ENABLE_SMP_
	this_current = (struct task_struct *)__get_cpu_var(current);
	this_sched_timer = (struct timer_struct *)__get_cpu_var(sched_timer);
	this_ptroot = (struct timer_root *)__get_cpu_var(ptroot);
#else
	this_current = current;
	this_sched_timer = sched_timer;
	this_ptroot = ptroot;
#endif

	// read banked PRIV_TMR_LD register.
	elapsed = (uint32_t)(readl(PRIV_TMR_LD));

	pct = container_of(this_ptroot->rb_leftmost, struct timer_struct, run_node);
	update_timer_tree(elapsed);
	pnt = container_of(this_ptroot->rb_leftmost, struct timer_struct, run_node);
	tc = pnt->tc;
	/* reprogram next earliest deadline timer intr. */
	writel(tc, PRIV_TMR_LD);

	update_current(elapsed);

	switch(pct->type) {
		case SCHED_TIMER:
			/* cfs task doesn't preempt rt task.
			 * Let's wait until rt task complete its
			 * task and yield().
			 */
			if (this_current->type == RT_TASK)
				break;

			this_sched_timer->handler(elapsed);
			pct->pt = this_current;
			break;

		case REALTIME_TIMER:
		case ONESHOT_TIMER:
			if (this_current != pct->pt) {
				if (this_current->type == RT_TASK)
					this_current->preempted = 1;

				switch_context(this_current, pct->pt);
				pct->pt->yield_task = this_current;
#if _ENABLE_SMP_
				__get_cpu_var(current) = pct->pt;
#else
				current = pct->pt;
#endif
			} 

			if (pct->type == ONESHOT_TIMER) {
				/* remove oneshot timer from timer tree */
				del_timer(this_ptroot, pct);
			} else if (!this_current->done && !this_current->preempted)
				/* missed the deadline
				 * when another timer intr 
				 * fires before rt task done and next 
				 * periodic timer. missed its deadline.
				 */
				this_current->missed_cnt++;

			break;

		default:
			break;
	}

	return 0;
}

uint32_t get_ticks_per_sec(void)
{
	return ticks_per_sec;
}

void set_ticks_per_sec(uint32_t tps)
{
	ticks_per_sec = tps;
}

#if 0
#define RT_TIMER_NUM		3

void rt_timer1(uint32_t el)
{
	xil_printf("rt timer 1!!\n");
}

void rt_timer2(uint32_t el)
{
	xil_printf("rt timer 2!!\n");
}

void rt_timer3(uint32_t el)
{
	xil_printf("rt timer 3!!\n");
}

timer_handler rt_timer_handler[RT_TIMER_NUM] = {
	rt_timer1,
	rt_timer2,
	rt_timer3,
};

void create_rt_timers(void)
{
	int i;
	for (i = 0; i < RT_TIMER_NUM; i++) {
		create_rt_timer(rt_timer_handler[i], 200 + 20 * i, i + 1, NULL);
	}
}
#endif

void init_timer(void)
{
	uint32_t tc = 0, cpuid = 0;
	struct timer_struct *pct = NULL;
	struct task_struct *this_current = NULL;
	struct timer_root *this_ptroot = NULL; 
#if _ENABLE_SMP_
	this_current = __get_cpu_var(current);
	this_ptroot = __get_cpu_var(ptroot);
#else
	this_current = current;
	this_ptroot = ptroot;
#endif
	/*init_timertree();*/
	/*create_sched_timer(cfs_sched_task, 10, 0, NULL);*/
	/*create_rt_timers();*/

	/*tc = sched_timer->tc;*/
	pct = container_of(this_ptroot->rb_leftmost, struct timer_struct, run_node);
	pct->pt = this_current;
	tc = pct->tc;
	writel(tc, PRIV_TMR_LD);
	/* register timer isr only from CPU 0.
	 * This is becasue CPU 0 and CPU 1 share the 
	 * same isr for its banked private timer intr.
	 * timer_irq doesn't need to be banked per cpu.
	 */
	cpuid = smp_processor_id();
	if (cpuid == 0)
		gic_register_int_handler(PRIV_TMR_INT_VEC, timer_irq, NULL);

	gic_mask_interrupt(PRIV_TMR_INT_VEC);
}


