/*
  kernel/inc/ktimer.h 
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

#ifndef _KTIMER_H_
#define _KTIMER_H_

#include <rbtree.h>
#include <stdint-gcc.h>
#include <task.h>

typedef void (*timer_handler)(uint32_t el);

struct clock_source_device {
	uint32_t current_tick;
	uint32_t current_time;
};

struct timer_root {
	struct rb_root root;
	struct rb_node *rb_leftmost;
};

enum timer_type {
	ONESHOT_TIMER = 0,
	SCHED_TIMER,
	REALTIME_TIMER,
};

struct timer_struct {
	uint32_t tc;
	uint32_t intvl;
	timer_handler handler;
	struct task_struct *pt;
	uint32_t type;
	struct rb_node run_node;
	void *arg;
};

void create_sched_timer(struct task_struct *cfs_sched_task, uint32_t msec, void *arg);
void create_rt_timer(struct task_struct *rt_task, uint32_t msec, void *arg);
void create_oneshot_timer(struct task_struct *oneshot_task, uint32_t msec, void *arg);
void init_timertree(void);
void update_csd(void);
void insert_timer(struct timer_root *ptr, struct timer_struct *pts);
void del_timer(struct timer_root *ptr, struct timer_struct *pts);
void sched_timer_handler(uint32_t elapsed);
void update_timer_tree(uint32_t elapsed);
uint32_t get_elapsedtime(void);
void init_oneshot_timers(void);
#endif
