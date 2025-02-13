/* SPDX-License-Identifier: MIT OR Apache-2.0
 *
 * Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>
 */

/**
 * @addtogroup kernel
 * @{
 * @addtogroup kernel_core Core
 * @{
 * @addtogroup kernel_core_proc Process management
 * @{
 *
 * @brief Kernel runq module
 *
 */

#ifndef _RUNQ_H_
#define _RUNQ_H_

#include <stdint.h>
#include <defs.h>
#include <sched.h>

/** CFS runq structure */
struct cfs_rq {
	struct sched_entity *curr;   /**< Pointer to current task's sched entity */
	struct sched_entity *next;   /**< Pointer to next task's sched entity */
	struct sched_entity *last;   /**< Pointer to last task's sched entity */
	struct rb_root root;	     /**< Runq root */
	struct rb_node *rb_leftmost; /**< Pointer to left most node in the runq */
	uint32_t priority_sum;	     /**< Priority sum of all task in runq */
	uint32_t cfs_task_num;	     /**< Task in CFS runq */
};

/**
 * @brief Initialize runq
 *
 */
void init_rq(void);

/**
 * @brief Dequeue sched entity from runq
 *
 * @param [in] se Pointer to sched entity of a task
 */
void dequeue_se(struct sched_entity *se);

/**
 * @brief Update virtual runtime of all task in the runq
 *
 * @param [in] se Pointer to sched entity of a task
 */
void update_vruntime_runq(struct sched_entity *se);

/**
 * @brief Update sched entity of current task
 *
 * @param elapsed Elapsed time
 */
void update_se(uint32_t elapsed);

/**
 * @brief Enqueue sched entity to runq
 *
 * @param [in] se Pointer to sched entity of a task
 */
void enqueue_se_to_runq(struct sched_entity *se);

#endif

/**
 * @}
 * @}
 * @}
 *
 */
