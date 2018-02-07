/*
  kernel/inc/task.h 
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
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef __TASK_H__
#define __TASK_H__

#include <stdbool.h>
#include <stdint-gcc.h>
#include <stddef.h>
#include <rbtree.h>
#include <mem_layout.h>
#include <mm.h>

#define MAX_TASK	((SVC_STACK_BASE-SYS_STACK_BASE)/(TASK_STACK_GAP))

enum {
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

struct cfs_rq {
	struct sched_entity *curr, *next, *last;
	struct rb_root root;
	struct rb_node *rb_leftmost;
	uint32_t priority_sum;
	uint32_t cfs_task_num;
};
/* do not change the order */
struct task_context_struct {
	uint32_t pc;
	uint32_t lr;
	uint32_t sp;
	uint32_t r[13];
	uint32_t spsr;
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
	uint32_t state;
	uint32_t timeinterval;
	struct mm_struct mm;
};

void init_rq(void);
void init_idletask(void);
void init_jiffies(void);
void init_cfs_workers(void);
void init_cfs_scheduler(void);
struct task_struct *forkyi(char *name, task_entry fn, TASKTYPE type);
void switch_context(struct task_struct *prev, struct task_struct *next);
void schedule(void);
void update_se(uint32_t elasped);
void dequeue_se_to_exit(struct sched_entity *se);
void enqueue_se_to_runq(struct sched_entity *se, bool update);
void dequeue_se_to_waitq(struct sched_entity *se, bool update);
void init_shell(void);
void init_workers(void);
void set_priority(struct task_struct *pt, uint32_t pri);
void yield(void);
void create_cfs_task(char *name, task_entry cfs_task, uint32_t pri);
void create_rt_task(char *name, task_entry handler, uint32_t dur);
void update_current(uint32_t elapsed);

static inline struct task_struct *to_task_from_listhead(struct list_head *t)
{       
	/*return container_of(t, struct task_struct, task);*/
	return ((struct task_struct *)((uint32_t)t-offsetof(struct task_struct, task)));
}

static inline struct task_struct *to_task_from_se(struct sched_entity *s)
{       
	/*return container_of(t, struct task_struct, task);*/
	return ((struct task_struct *)((uint32_t)s-offsetof(struct task_struct, se)));
}
#endif
