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

#include <stdlib.h>
#include <ktimer.h>
#include <timer.h>
#include <defs.h>
#include <mm.h>
#include <xil_printf.h>
#include <regops.h>
#include <task.h>

struct clock_source_device csd;
struct timer_root *ptroot = NULL;
struct timer_struct *sched_timer = NULL;
uint64_t jiffies;
volatile uint32_t timertree_lock;

#define MIN_TIME_INT 	(get_ticks_per_sec() / 1000)
#define MSEC_MARGIN	(get_ticks_per_sec() / 1000)

void init_timertree(void)
{
	ptroot = (struct timer_root *) kmalloc(sizeof(struct timer_root));
	ptroot->root = RB_ROOT;
}

/* need to use 64bit Global Timer */
void update_csd(void)
{
	csd.current_tick = timer_get_phy_tick_cnt();
	/*csd.current_time = (uint64_t)(csd.current_tick / get_ticks_per_sec());*/
}

uint32_t get_elapsedtime(void)
{
	return (uint32_t)(readl(PRIV_TMR_LD));
}

void update_timer_tree(uint32_t elapsed)
{
	uint32_t temp;
	struct rb_node *pcur = NULL;
	struct timer_struct *pct = NULL;

	pcur = ptroot->rb_leftmost;
	/* update timer tree according to elapsed value */
	while (pcur != NULL) {
		pct = container_of(pcur, struct timer_struct, run_node);
		if (pcur == ptroot->rb_leftmost) {
			pct->tc = pct->intvl;
		} else {
			/* deadline has a margin since the timer intr
			   has some overhead
			 */
			/* if miss the deadline with margin */
			if (pct->tc + MSEC_MARGIN <= elapsed) {
				if (pct->type == REALTIME_TIMER) {
					pct->pt->missed_cnt++;
				} 
				pct->tc = MIN_TIME_INT;
			} else {
				temp = (pct->tc - elapsed); 
				pct->tc = temp > MIN_TIME_INT ? temp : MIN_TIME_INT;
			}
		}
		pcur = rb_next(pcur);
	}

	/* Update the location of the left-most node only.
	   All other nodes are already sorted and
	   updated only the tc value with elapsed time.
	 */
	pcur = ptroot->rb_leftmost;
	pct = container_of(pcur, struct timer_struct, run_node);
	del_timer(ptroot, pct);
	insert_timer(ptroot, pct);
}

void do_sched_timer(uint32_t elapsed)
{
	update_se(elapsed);
	schedule();
}

void cfs_scheduler(uint32_t elapsed)
{
	update_se(elapsed);
	schedule();
}

void create_rt_timer(struct task_struct *rt_task, 
		uint32_t msec, uint32_t idx, void *arg)
{
	struct timer_struct *rt_timer = (struct timer_struct *)kmalloc(sizeof(struct timer_struct));
	rt_timer->pt = rt_task;
	rt_timer->handler = NULL;
	rt_timer->type = REALTIME_TIMER;
	rt_timer->tc = get_ticks_per_sec() / 1000 * msec;
	rt_timer->intvl = rt_timer->tc;
	rt_timer->idx = idx;
	rt_timer->arg = arg;

	insert_timer(ptroot, rt_timer);
}

void create_oneshot_timer(struct task_struct *oneshot_task, 
		uint32_t msec, uint32_t idx, void *arg)
{
	struct timer_struct *oneshot_timer = (struct timer_struct *)kmalloc(sizeof(struct timer_struct));
	oneshot_timer->pt = oneshot_task;
	oneshot_timer->handler = NULL;
	oneshot_timer->type = ONESHOT_TIMER;
	oneshot_timer->tc = get_ticks_per_sec() / 1000 * msec;
	oneshot_timer->intvl = oneshot_timer->tc;
	oneshot_timer->idx = idx;
	oneshot_timer->arg = arg;

	insert_timer(ptroot, oneshot_timer);
}

void create_sched_timer(struct task_struct *cfs_sched_task, 
		uint32_t msec, uint32_t idx, void *arg)
{
	sched_timer = (struct timer_struct *)kmalloc(sizeof(struct timer_struct));
	sched_timer->pt = cfs_sched_task;
	sched_timer->handler = cfs_scheduler;
	sched_timer->type = SCHED_TIMER;
	/* 10msec sched timer tick */
	sched_timer->tc = get_ticks_per_sec() / 1000 * msec;
	sched_timer->intvl = sched_timer->tc;
	sched_timer->idx = idx;
	sched_timer->arg = arg;

	insert_timer(ptroot, sched_timer);
}

void insert_timer(struct timer_root *ptr, struct timer_struct *pts)
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

void del_timer(struct timer_root *ptr, struct timer_struct *pts)
{
	if (ptr->rb_leftmost == (struct rb_node *)&pts->run_node) {
		struct rb_node *next_node;

		next_node = rb_next(&pts->run_node);
		ptr->rb_leftmost = next_node;
	}

	rb_erase(&pts->run_node, &ptr->root);
}

