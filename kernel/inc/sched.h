/*
  kernel/inc/sched.h 
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

#ifndef __SCHED_H__
#define __SCHED_H__
#include <stdint-gcc.h>
#include <mem_layout.h>
#include <defs.h>
#include <rbtree.h>
#include <mm.h>

#define MAX_TASK	((SVC_STACK_BASE-SYS_STACK_BASE)/(TASK_STACK_GAP))

enum task_state {
	TASK_RUNNING,
	TASK_WAITING,
	TASK_STOP_RUNNING,
	TASK_STOP
};

struct list_head {
	struct list_head *prev, *next;
};


typedef uint32_t (*task_entry)(void);

struct sched_entity {
	/*uint64_t ticks_vruntime;*/
	uint32_t jiffies_vruntime;
	uint64_t ticks_consumed;
	uint32_t jiffies_consumed;
	struct rb_node run_node;
	uint32_t priority;
};

/* do not change the order */
struct task_context_struct {
	uint32_t r[12];
	uint32_t sp;
	uint32_t lr;
	uint32_t pc;
};

typedef enum {
	CFS_TASK = 0,
	RT_TASK,
	ONESHOT_TASK,
}TASKTYPE;

struct task_struct {
	struct task_context_struct ct;
	task_entry entry;
	void *arg;
	char name[32];
	uint32_t pid;
	struct sched_entity se;
	struct list_head task;
	struct task_struct *yield_task;
	struct list_head waitlist;
	TASKTYPE type; 
	uint32_t missed_cnt;
	enum task_state state;
	uint32_t timeinterval;
	struct mm_struct mm;
	uint32_t done;
	uint32_t preempted;
};

static inline struct task_struct *to_task_from_listhead(struct list_head *t)
{       
	/*return container_of(t, struct task_struct, task);*/
	return ((struct task_struct *)((uint32_t)t - offsetof(struct task_struct, task)));
}

static inline struct task_struct *to_task_from_se(struct sched_entity *s)
{       
	/*return container_of(t, struct task_struct, task);*/
	return ((struct task_struct *)((uint32_t)s - offsetof(struct task_struct, se)));
}

extern void do_switch_context(struct task_struct *, struct task_struct *);
extern void switch_context_yield(struct task_struct *, struct task_struct *);
void init_cfs_scheduler(void);
void init_jiffies(void);
struct task_struct *forkyi(char *name, task_entry fn, TASKTYPE type);
void switch_context(struct task_struct *prev, struct task_struct *next);
void schedule(void);
void set_priority(struct task_struct *pt, uint32_t pri);
void yield(void);
void update_current(uint32_t elapsed);
void print_task_stat(void *);
#endif
