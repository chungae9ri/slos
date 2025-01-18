// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

#ifndef _SCHED_H_
#define _SCHED_H_

#include <stdint.h>
#include <mem_layout.h>
#include <defs.h>
#include <rbtree.h>
#include <mm.h>

#define MAX_TASK    ((SVC_STACK_BASE - SYS_STACK_BASE) / (TASK_STACK_GAP))
#define CFS_PRI_NUM (5u)

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
	uint32_t pri_div_shift;
};

/* do not change the order */
struct task_context_struct {
	/* TODO: task ctx has r0-r11, r12 isn't included */
	uint32_t r[12];
	uint32_t sp;
	uint32_t lr;
	uint32_t pc;
	uint32_t reserved; /* needed to align 8bytes for d0-15 registers */
	uint64_t d[16];    /* 16 double-precision registers (d0-d15) */
	uint32_t fpscr;    /* Floating-Point Status and Control Register */
	uint32_t fpexc;    /* Floating-Point Exception Register */
};

typedef enum {
	CFS_TASK = 0,
	RT_TASK,
	ONESHOT_TASK,
} TASKTYPE;

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
struct task_struct *forkyi(char *name, task_entry fn, TASKTYPE type, uint32_t pri);
void switch_context(struct task_struct *prev, struct task_struct *next);
void schedule(void);
void yield(void);
void update_current(uint32_t elapsed);
void print_task_stat(void *);
#endif
