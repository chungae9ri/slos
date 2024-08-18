// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

#ifndef _WAITQ_H_
#define _WAITQ_H_

#include <stdint.h>
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

#endif