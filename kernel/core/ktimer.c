/*
  kernel/core/ktimer.c kernel timer
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

#include <ktimer.h>
#include <timer.h>
#include <defs.h>
#include <mm.h>
#include <regops.h>
#include <sched.h>
#include <runq.h>
#include <percpu.h>

#define MAX_ONESHOT_TIMER_NUM		32
#define MIN_TIME_INT 	(get_ticks_per_sec() >> 10)
#define MSEC_MARGIN	(get_ticks_per_sec() >> 10)

static void cfs_scheduler(uint32_t elapsed)
{
	update_se(elapsed);
	schedule();
}

static void insert_timer(struct timer_root *ptr, struct timer_struct *pts)
{
	struct rb_node **link = &ptr->root.rb_node, *parent = NULL;
	uint64_t value = pts->tc;
	int leftmost = 1;

	/* Go to the bottom of the tree */
	while (*link) {
		parent = *link;
		struct timer_struct *entry = rb_entry(parent, struct timer_struct, run_node);
		if (entry->tc > value)
			link = &(*link)->rb_left;
		else /* if (entry->tc<= value)*/ {
			link = &(*link)->rb_right;
			leftmost = 0;
		}
	}
	/* Maintain a cache of leftmost tree entries */

	if (leftmost)
		ptr->rb_leftmost = &pts->run_node;
	/* put the new node there */
	rb_link_node(&pts->run_node, parent, link);
	rb_insert_color(&pts->run_node, &ptr->root);
}

void init_timertree(void)
{
	struct timer_root *this_ptroot;
#if _ENABLE_SMP_
	__get_cpu_var(csd) = (struct clock_source_device *)kmalloc(sizeof(struct clock_source_device));
	__get_cpu_var(ptroot) = (struct timer_root *) kmalloc(sizeof(struct timer_root));
	this_ptroot = (struct timer_root *)__get_cpu_var(ptroot);
#else
	csd = (struct clock_source_device *)kmalloc(sizeof(struct clock_source_device));
	ptroot = (struct timer_root *) kmalloc(sizeof(struct timer_root));
	this_ptroot = ptroot;
#endif
	this_ptroot->root = RB_ROOT;
	this_ptroot->rb_leftmost = NULL;
}

/* need to use 64bit Global Timer */
void update_csd(void)
{
	struct clock_source_device *pthis_csd;
#if _ENABLE_SMP_
	pthis_csd = __get_cpu_var(csd);
#else
	pthis_csd = csd;
#endif
	pthis_csd->current_tick = timer_get_phy_tick_cnt();
}

uint32_t get_elapsedtime(void)
{
	return (uint32_t)(readl(PRIV_TMR_LD));
}

void update_timer_tree(uint32_t elapsed)
{
	struct timer_root *this_ptroot;
	uint32_t temp;
	struct rb_node *pcur = NULL;
	struct timer_struct *pct = NULL;

#if _ENABLE_SMP_
	this_ptroot = __get_cpu_var(ptroot);
#else
	this_ptroot = ptroot;
#endif
	pcur = this_ptroot->rb_leftmost;
	/* update timer tree according to elapsed value */
	while (pcur != NULL) {
		pct = container_of(pcur, struct timer_struct, run_node);
		if (pcur == this_ptroot->rb_leftmost) {
			if (pct->pt->done) {
				pct->tc = pct->intvl;
			} else {
				/* re-enqueue this rt task to finish
				 * its work in 1msec
				 */
				temp = (pct->tc - elapsed); 
				if (temp < MIN_TIME_INT) 
					pct->tc = MIN_TIME_INT;
				else 
					pct->tc = temp;
			}
		} else {
			temp = (pct->tc - elapsed); 
			if (temp < MIN_TIME_INT) 
				pct->tc = MIN_TIME_INT;
			else 
				pct->tc = temp;
		}
		pcur = rb_next(pcur);
	}

	/* Update the location of the left-most node only.
	   All other nodes are already sorted and
	   updated only the tc value with elapsed time.
	 */
	pcur = this_ptroot->rb_leftmost;
	pct = container_of(pcur, struct timer_struct, run_node);
	del_timer(this_ptroot, pct);
	insert_timer(this_ptroot, pct);
}

void create_rt_timer(struct task_struct *rt_task, uint32_t msec, void *arg)
{
	struct timer_root *this_ptroot;
	struct timer_struct *rt_timer = (struct timer_struct *)kmalloc(sizeof(struct timer_struct));

	rt_timer->pt = rt_task;
	rt_timer->handler = NULL;
	rt_timer->type = REALTIME_TIMER;
	rt_timer->tc = get_ticks_per_sec() / 1000 * msec;
	rt_timer->intvl = rt_timer->tc;
	rt_timer->arg = arg;

#if _ENABLE_SMP_
	this_ptroot = __get_cpu_var(ptroot);
#else
	this_ptroot = ptroot;
#endif
	insert_timer(this_ptroot, rt_timer);
}

void init_oneshot_timers(void)
{
	int i;
	uint32_t *pthis_oneshot_timer_idx;
	struct timer_struct *this_oneshot_timer;

	this_oneshot_timer = (struct timer_struct *)kmalloc(sizeof(struct timer_struct) * MAX_ONESHOT_TIMER_NUM);
#if _ENABLE_SMP_
	__get_cpu_var(oneshot_timer) = this_oneshot_timer;
	pthis_oneshot_timer_idx = (uint32_t *)__get_cpu_var_addr(oneshot_timer_idx);
#else
	oneshot_timer = this_oneshot_timer;
	pthis_oneshot_timer_idx = &oneshot_timer_idx;
#endif
	*pthis_oneshot_timer_idx = 0;

	for (i = 0; i < MAX_ONESHOT_TIMER_NUM; i++) {
		this_oneshot_timer->pt = NULL;
		this_oneshot_timer->handler = NULL;
		this_oneshot_timer->type = ONESHOT_TIMER;
		this_oneshot_timer->tc = 0;
		this_oneshot_timer->intvl = 0;
		this_oneshot_timer->arg = NULL;
		this_oneshot_timer++;
	}
}

void create_oneshot_timer(struct task_struct *oneshot_task, uint32_t tc, void *arg)
{
	uint32_t *pthis_oneshot_timer_idx;
	struct timer_root *this_ptroot;
	struct timer_struct *this_oneshot_timer;

#if _ENABLE_SMP_
	this_ptroot = (struct timer_root *)__get_cpu_var(ptroot);
	this_oneshot_timer = (struct timer_struct *)__get_cpu_var(oneshot_timer);
	pthis_oneshot_timer_idx = (uint32_t *)__get_cpu_var_addr(oneshot_timer_idx);
#else
	this_ptroot = ptroot;
	this_oneshot_timer = oneshot_timer;
	pthis_oneshot_timer_idx = &oneshot_timer_idx;
#endif

	this_oneshot_timer[*pthis_oneshot_timer_idx].pt = oneshot_task;
	this_oneshot_timer[*pthis_oneshot_timer_idx].handler = NULL;
	this_oneshot_timer[*pthis_oneshot_timer_idx].type = ONESHOT_TIMER;
	this_oneshot_timer[*pthis_oneshot_timer_idx].tc = tc;
	this_oneshot_timer[*pthis_oneshot_timer_idx].intvl = tc;
	/* idx is inited in init_oneshot_timers and 
	 * not used in oneshot timer. 
	 */
	this_oneshot_timer[*pthis_oneshot_timer_idx].arg = arg;
	insert_timer(this_ptroot, &this_oneshot_timer[*pthis_oneshot_timer_idx]);

	(*pthis_oneshot_timer_idx)++;
	if (*pthis_oneshot_timer_idx == MAX_ONESHOT_TIMER_NUM)
		*pthis_oneshot_timer_idx = 0;
}

void create_sched_timer(struct task_struct *cfs_sched_task, uint32_t msec, void *arg)
{
	struct timer_struct *this_sched_timer = NULL;
	struct timer_root *this_ptroot = NULL;

#if _ENABLE_SMP_
	__get_cpu_var(sched_timer) = (struct timer_struct *)kmalloc(sizeof(struct timer_struct));
	this_sched_timer = __get_cpu_var(sched_timer);
	this_ptroot = __get_cpu_var(ptroot);
#else
	sched_timer = (struct timer_struct *)kmalloc(sizeof(struct timer_struct));
	this_sched_timer = sched_timer;
	this_ptroot = ptroot;
#endif
	if (this_ptroot->root.rb_node == (struct rb_node *)0xffffffff)
		this_ptroot->root.rb_node = 0;

	this_sched_timer->pt = cfs_sched_task;
	this_sched_timer->handler = cfs_scheduler;
	this_sched_timer->type = SCHED_TIMER;
	/* 10msec sched timer tick */
	this_sched_timer->tc = get_ticks_per_sec() / 1000 * msec;
	this_sched_timer->intvl = this_sched_timer->tc;
	this_sched_timer->arg = arg;

	insert_timer(this_ptroot, this_sched_timer);
}

void del_timer(struct timer_root *ptr, struct timer_struct *pts)
{
	/* oneshot timer doesn't need to be deleted.
	 * update rb timer tree only.
	 * Other timers' resource that was kmalloc-ed
	 * isn't free-ed for now.
	 */
	if (ptr->rb_leftmost == (struct rb_node *)&pts->run_node) {
		struct rb_node *next_node;

		next_node = rb_next(&pts->run_node);
		ptr->rb_leftmost = next_node;
	}

	rb_erase(&pts->run_node, &ptr->root);
}

