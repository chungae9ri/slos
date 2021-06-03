/*
  kernel/inc/wait.h 
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

#include <stdint-gcc.h>
#include <sched.h>
#include <stdbool.h>

struct wait_queue {
	int magic;
	volatile uint32_t wqlock;
	struct list_head task_list;
	int count;
};

extern void spin_lock_acquire(volatile uint32_t *pl);
extern void spin_lock_release(volatile uint32_t *pl);

void init_wq(void);
void add_to_wq(struct task_struct *tp);
void dequeue_se_to_wq(struct sched_entity *se);
void remove_from_wq(struct task_struct *tp);
void dequeue_se_to_exit(struct sched_entity *se);
void dequeue_se_to_waitq(struct sched_entity *se, bool update);
