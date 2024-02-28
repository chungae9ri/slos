// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

#include <stdint.h>
#include <timer.h>
#include <gic.h>
#include <xparameters_ps.h>
#include <regops.h>
#include <rbtree.h>
#include <ktimer.h>
#include <defs.h>
#include <percpu.h>
#include <waitq.h>
#include <runq.h>

extern uint32_t smp_processor_id();
static uint32_t ticks_per_sec;

static void delay(uint32_t ticks)
{
	struct task_struct *this_current = NULL;
	struct task_struct *this_idle_task = NULL;

#if _ENABLE_SMP_
	this_current = (struct task_struct *)__get_cpu_var(current);
	this_idle_task = (struct task_struct *)__get_cpu_var(idle_task);
#else
	this_current = current;
	this_idle_task = idle_task;
#endif

	dequeue_se_to_wq(&this_current->se);
	create_oneshot_timer(this_current, ticks, NULL);
	if (this_current->yield_task->state == TASK_WAITING)
		this_current->yield_task = this_idle_task;

	yield();
}

void msleep(unsigned msecs)
{
	uint64_t ticks;
	/* divide by 1024 */
	ticks = ((uint64_t)msecs * ticks_per_sec) >> 10;
	delay(ticks);
}

void usleep(unsigned usecs)
{
	uint64_t ticks;
	/* divide by 1024 * 1024 */
	ticks = ((uint64_t)usecs * ticks_per_sec) >> 20;
	delay(ticks);
}

inline uint32_t get_timer_freq()
{
	return (XPAR_CPU_CORTEXA9_0_CPU_CLK_FREQ_HZ >> 1);
}

inline uint32_t timer_get_phy_tick_cnt(void)
{
	return (uint32_t)readl(PRIV_TMR_CNTR);
}

/* banked cpu private timer: cpu0 */
void timer_enable(void)
{
	int ctrl;

	ctrl = readl(PRIV_TMR_CTRL);
	ctrl = ctrl | (PRIV_TMR_EN_MASK 
			| PRIV_TMR_AUTO_RE_MASK
			| PRIV_TMR_IRQ_EN_MASK);
		
	writel(ctrl, PRIV_TMR_CTRL);
}

/* banked cpu private timer: cpu1 */
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

				/* oneshot, realtime timer task should 
				 * be switched everytime.
				 */
				switch_context(this_current, pct->pt);
				pct->pt->yield_task = this_current;
#if _ENABLE_SMP_
				__get_cpu_var(current) = pct->pt;
#else
				current = pct->pt;
#endif
			} 

			if (pct->type == ONESHOT_TIMER) {
				if (pct->pt->state == TASK_WAITING) {
					enqueue_se_to_runq(&pct->pt->se);
				}
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

void init_timer(void)
{
	uint32_t tc = 0;
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
	pct = container_of(this_ptroot->rb_leftmost, struct timer_struct, run_node);
	pct->pt = this_current;
	tc = pct->tc;
	writel(tc, PRIV_TMR_LD);

	/* register timer isr for each corresponding cpu in
	 * the gic_register_int_handler()
	 */
	gic_register_int_handler(PRIV_TMR_INT_VEC, timer_irq, NULL);
	gic_mask_interrupt(PRIV_TMR_INT_VEC);
}


