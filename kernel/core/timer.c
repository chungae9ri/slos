// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

/**
 * @addtogroup kernel
 * @{
 * @addtogroup kernel_core Core
 * @{
 * @addtogroup kernel_core_proc Process management
 * @{
 *
 * @file
 *
 * @brief Timer event device including delay functions, timer interrupt handler
 *
 */

#include <generated_kconfig_defs.h>

#define DEVICE_DT_COMPAT ARM_PRIVATE_TIMER

#if defined(ARCH_CORTEX_A9)

#include <stdint.h>

#include <timer.h>
#include <gic_v1.h>
#include <rbtree.h>
#include <ktimer.h>
#include <defs.h>
#include <percpu.h>
#include <waitq.h>
#include <runq.h>
#include <ops.h>
#include <generated_devicetree_defs.h>

/** Define GIC device from devicetree */
DEVICE_DEFINE_IDX(timer, 0);

/** Ticks per second */
static uint32_t ticks_per_sec;

/**
 * @brief Nonblocking delay current task for ticks.
 *
 * This suspend current task and yield cpu to next task.
 *
 * @param [in] ticks Delay duration
 */
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

	yieldyi();
}

void mdelay(uint32_t msecs)
{
	uint64_t ticks;
	/* divide by 1024 */
	ticks = ((uint64_t)msecs * ticks_per_sec) >> 10;
	delay(ticks);
}

int32_t timer_enable(const struct device *dev)
{
	int ctrl;

	if (dev == NULL) {
		return -EINVAL;
	}

	ctrl = read32(dev->base_addr + PRIV_TMR_CTRL_OFFSET);
	ctrl = ctrl | (PRIV_TMR_EN_MASK | PRIV_TMR_AUTO_RE_MASK | PRIV_TMR_IRQ_EN_MASK);

	write32(dev->base_addr + PRIV_TMR_CTRL_OFFSET, ctrl);

	return 0;
}

/* banked cpu private timer: cpu1 */
void timer_enable_secondary(const struct device *dev)
{
	uint32_t ctrl;

	/* Init timer */
	*(uint32_t *)(dev->base_addr + PRIV_TMR_LD_OFFSET) = 1000000;

	/* Enable timer */
	ctrl = read32(dev->base_addr + PRIV_TMR_CTRL_OFFSET);
	ctrl = ctrl | (PRIV_TMR_EN_MASK | PRIV_TMR_AUTO_RE_MASK | PRIV_TMR_IRQ_EN_MASK);

	write32(dev->base_addr + PRIV_TMR_CTRL_OFFSET, ctrl);
}

int32_t timer_disable(const struct device *dev)
{
	int ctrl;

	if (dev == NULL) {
		return -EINVAL;
	}

	ctrl = read32(dev->base_addr + PRIV_TMR_CTRL_OFFSET);
	ctrl = ctrl & ~PRIV_TMR_EN_MASK;

	write32(dev->base_addr + PRIV_TMR_CTRL_OFFSET, ctrl);

	return 0;
}

int32_t timer_irq(void *arg)
{
	uint32_t elapsed = 0;
	uint32_t tc = 0;
	struct timer_struct *pnt = NULL, *pct = NULL;
	struct task_struct *this_current = NULL;
	struct timer_struct *this_sched_timer = NULL;
	struct timer_root *this_ptroot;
	struct device *dev = (struct device *)arg;

	if (dev == NULL) {
		return -EINVAL;
	}

#if _ENABLE_SMP_
	this_current = (struct task_struct *)__get_cpu_var(current);
	this_sched_timer = (struct timer_struct *)__get_cpu_var(sched_timer);
	this_ptroot = (struct timer_root *)__get_cpu_var(ptroot);
#else
	this_current = current;
	this_sched_timer = sched_timer;
	this_ptroot = ptroot;
#endif

	/* Read banked PRIV_TMR_LD register. */
	elapsed = (uint32_t)(read32(dev->base_addr + PRIV_TMR_LD_OFFSET));

	pct = container_of(this_ptroot->rb_leftmost, struct timer_struct, run_node);
	update_timer_tree(elapsed);
	pnt = container_of(this_ptroot->rb_leftmost, struct timer_struct, run_node);
	tc = pnt->tc;
	/* Reprogram next earliest deadline timer intr. */
	write32(dev->base_addr + PRIV_TMR_LD_OFFSET, tc);

	update_current(elapsed);

	switch (pct->type) {
	case SCHED_TIMER:
		/* cfs task doesn't preempt rt task.
		 * Let's wait until rt task complete its
		 * task and yieldyi().
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

			/* Oneshot, realtime timer task should
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
			/* Remove oneshot timer from timer tree */
			del_timer(this_ptroot, pct);
		} else if (!this_current->done && !this_current->preempted)
			/* Missed the deadline
			 * when another timer intr
			 * fires before rt task done and next
			 * periodic timer. Missed its deadline.
			 */
			this_current->missed_cnt++;

		break;

	default:
		break;
	}

	/* Clear timer interrupt status */
	write32(dev->base_addr + PRIV_TMR_INTSTAT_OFFSET, 1);

	return 0;
}

uint32_t get_ticks_per_sec(void)
{
	if (ticks_per_sec == 0) {
		/* If ticks_per_sec is not set, read from device tree*/
		ticks_per_sec = DT_GET_CLKFREQ(0);
	}

	return ticks_per_sec;
}

int32_t init_timer(struct device *dev)
{
	uint32_t tc = 0;
	struct timer_struct *pct = NULL;
	struct task_struct *this_current = NULL;
	struct timer_root *this_ptroot = NULL;

	if (dev == NULL) {
		return -EINVAL;
	}

	dev->name = DT_GET_COMPAT(0);
	dev->base_addr = DT_GET_BASE_ADDR(0);
	dev->irq = DT_GET_IRQ(0);
	dev->data = NULL;
	ticks_per_sec = DT_GET_CLKFREQ(0);

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
	write32(dev->base_addr + PRIV_TMR_LD_OFFSET, tc);

	return 0;
}

#else

#include <stdint.h>
#include <stddef.h>

#include <timer.h>
#include <gic_v2.h>
#include <generated_devicetree_defs.h>

/** Define GIC device from devicetree */
DEVICE_DEFINE_IDX(timer, 0);

/** Ticks per second */
static uint32_t ticks_per_sec;

int32_t timer_irq(void *arg)
{
	return 0;
}

int32_t init_timer(struct device *dev)
{
	if (dev == NULL) {
		return -EINVAL;
	}

	dev->name = DT_GET_COMPAT(0);
	dev->base_addr = DT_GET_BASE_ADDR(0);
	dev->irq = DT_GET_IRQ(0);
	dev->data = NULL;
	ticks_per_sec = DT_GET_CLKFREQ(0);

	return 0;
}
#endif

/**
 * @}
 * @}
 * @}
 *
 */
