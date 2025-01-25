/* SPDX-License-Identifier: MIT OR Apache-2.0
 *
 * Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>
 */

#ifndef _RUNQ_H_
#define _RUNQ_H_

#include <stdint.h>
#include <defs.h>
#include <sched.h>

struct cfs_rq {
	struct sched_entity *curr, *next, *last;
	struct rb_root root;
	struct rb_node *rb_leftmost;
	uint32_t priority_sum;
	uint32_t cfs_task_num;
};

void init_rq(void);
void dequeue_se(struct sched_entity *se);
void update_vruntime_runq(struct sched_entity *se);
void update_se(uint32_t elapsed);

void enqueue_se_to_runq(struct sched_entity *se);

#endif
