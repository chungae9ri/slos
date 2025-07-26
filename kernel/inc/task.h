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
 * @brief Task creation including shell task
 *
 */

#ifndef _TASK_H_
#define _TASK_H_

#include <stdint.h>
#include <stddef.h>

#include <mem_layout.h>
#include <sched.h>

/** Max workq length */
#define MAX_WORKQ 32

/** Flag to print current task name */
extern uint32_t show_stat;

/** Workq entry structure */
struct workq {
	int32_t (*func)(void *arg);
	void *arg;
};

/** Worker task structure */
struct worker {
	int enq_idx;
	int deq_idx;
	struct task_struct *task;
	struct workq wkq[MAX_WORKQ];
};

/**
 * @brief Initialize idle task
 *
 */
void init_idletask(void);

/**
 * @brief Initialize shell task
 *
 */
void init_shell(void);

/**
 * @brief Create a usr task object
 *
 * @param [in] name Task name
 * @param [in] cfs_task User task entry
 * @param [in] pri Priority
 * @param [in] appIdx Task application index
 * @return struct task_struct* User task object
 */
struct task_struct *create_usr_cfs_task(char *name, task_entry cfs_task, uint32_t pri,
					uint32_t appIdx);

/**
 * @brief Create a cfs task object
 *
 * @param [in] name CFS task name
 * @param [in] cfs_task Task entry
 * @param [in] pri Task priority
 * @param [in] arg Task argument
 * @return struct task_struct* CFS task object
 */
struct task_struct *create_cfs_task(char *name, task_entry cfs_task, uint32_t pri, void *arg);

/**
 * @brief Create a rt task object
 *
 * @param [in] name RT task name
 * @param [in] handler RT task entry
 * @param [in] dur RT task deadline
 * @return struct task_struct* RT task object
 */
struct task_struct *create_rt_task(char *name, task_entry handler, uint32_t dur);

/**
 * @brief Create a workq worker object
 *
 */
void create_workq_worker(void);

/**
 * @brief Enqueue a work function to workq
 *
 * @param [in] func Work function
 * @param [in] arg Work function argument
 * @return int32_t 0 for success
 */
int32_t enqueue_workq(int32_t (*func)(void *), void *arg);

/**
 * @brief Enqueue workq worker task to runq
 *
 */
void wakeup_workq_worker(void);
#endif

/**
 * @}
 * @}
 * @}
 *
 */
