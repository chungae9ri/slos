// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

#ifndef _TASK_H_
#define _TASK_H_

#include <stdint.h>
#include <stddef.h>
#include <mem_layout.h>
#include <sched.h>

#define MAX_WORKQ 32
#define WORKQ_DMA 0

struct workq {
	void (*func)(void *);
	void *arg;
};

struct worker {
	int enq_idx;
	int deq_idx;
	struct task_struct *task;
	struct workq wkq[MAX_WORKQ];
};

void init_idletask(void);
void init_cfs_workers(void);
void init_shell(void);
void init_workers(void);

struct task_struct *create_usr_cfs_task(char *name, task_entry cfs_task, uint32_t pri,
                                        uint32_t appIdx);
struct task_struct *create_cfs_task(char *name, task_entry cfs_task, uint32_t pri);
struct task_struct *create_rt_task(char *name, task_entry handler, uint32_t dur);
void create_workq_worker(void);
uint32_t enqueue_workq(void (*func)(void *), void *arg);
void wakeup_workq_worker(void);
#endif
