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
 * @brief Kernel waitq module
 *
 */

#ifndef _WAITQ_H_
#define _WAITQ_H_

#include <stdint.h>
#include <stdbool.h>

#include <sched.h>

/** Waitq structure */
struct wait_queue {
	int magic;
	uint32_t wqlock;
	struct list_head task_list;
	int count;
};

/**
 * @brief Initialize waitq
 *
 */
void init_wq(void);

/**
 * @brief Add task to waitq
 *
 * @param [in] tp Pointer to task object
 */
void add_to_wq(struct task_struct *tp);

/**
 * @brief Dequeue sched entity from runq to waitq
 *
 * @param [in] se Sched entity
 */
void dequeue_se_to_wq(struct sched_entity *se);

/**
 * @brief Remove task from waitq
 *
 * @param [in] tp Pointer to task
 */
void remove_from_wq(struct task_struct *tp);

/**
 * @brief Exit task from waitq - not working
 *
 * @param [in] se Sched entity
 */
void dequeue_se_to_exit(struct sched_entity *se);

#endif

/**
 * @}
 * @}
 * @}
 *
 */
