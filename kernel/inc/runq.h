/*
  kernel/inc/runq.h 
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

#ifndef __RUNQ_H__
#define __RUNQ_H__
#include <stdbool.h>
#include <stdint-gcc.h>
#include <defs.h>
#include <sched.h>

struct cfs_rq {
	struct sched_entity *curr, *next, *last;
	struct rb_root root;
	struct rb_node *rb_leftmost;
	uint32_t priority_sum;
	uint32_t cfs_task_num;
};


extern void spin_lock_acquire(volatile uint32_t *pl);
extern void spin_lock_release(volatile uint32_t *pl);

void init_rq(void);
void enqueue_se(struct sched_entity *se);
void dequeue_se(struct sched_entity *se);
void update_vruntime_runq(struct sched_entity *se);
void update_se(uint32_t elapsed);

void enqueue_se_to_runq(struct sched_entity *se, bool update);

#endif
