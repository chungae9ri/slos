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
 * @brief Kernel timer framework
 *
 */

#ifndef _KTIMER_H_
#define _KTIMER_H_

#include <rbtree.h>
#include <stdint.h>
#include <task.h>

/** Timer handler type definition */
typedef void (*timer_handler)(uint32_t el);

/** Clock source device */
struct clock_source_device {
	uint32_t current_tick; /**< Current timer tick count - not used */
	uint32_t current_time; /**< Current time - not used  */
};

/** Timer rbtree root */
struct timer_root {
	struct rb_root root;	     /**< Timer rbtree root node */
	struct rb_node *rb_leftmost; /**< Timer rbtree leftmost node */
};

/** Timer types */
enum timer_type {
	ONESHOT_TIMER = 0, /**< Oneshot timer */
	SCHED_TIMER,	   /**< CFS scheduler timer */
	REALTIME_TIMER,	   /**< Realtime timer */
};

/** Timer struct definition */
struct timer_struct {
	uint32_t tc;		 /**< Tick count */
	uint32_t intvl;		 /**< Timer interval of next timer interrupt */
	timer_handler handler;	 /**< Timer handler */
	struct task_struct *pt;	 /**< Task struct associated with timer */
	uint32_t type;		 /**< Timer type */
	struct rb_node run_node; /**< Current timer rbnode in timer rbtree */
	void *arg;		 /**< Timer argument */
};

/**
 * @brief Create a sched timer object
 *
 * @param [in] cfs_sched_task Task associated with this timer
 * @param [in] msec Timeout (interval) of timer
 * @param [in] arg Argument for the task
 */
void create_sched_timer(struct task_struct *cfs_sched_task, uint32_t msec, void *arg);

/**
 * @brief Create a rt timer object
 *
 * @param [in] rt_task Realtime task associated with this realtime timer
 * @param [in] msec Realtime timer interval
 * @param [in] arg Argument to the task
 */
void create_rt_timer(struct task_struct *rt_task, uint32_t msec, void *arg);

/**
 * @brief Create a oneshot timer object
 *
 * @param [in] oneshot_task Oneshot task associated with this oneshot timer
 * @param [in] msec Oneshot timer interval
 * @param [in] arg Argument to the task
 */
void create_oneshot_timer(struct task_struct *oneshot_task, uint32_t msec, void *arg);

/**
 * @brief Initialize timer rbtree
 *
 */
void init_timertree(void);

/**
 * @brief Update current timer tick count
 *
 */
void update_csd(void);

/**
 * @brief Delete timer from timer rbtree
 *
 * @param [in] ptr Timer rbtree root
 * @param [in] pts Timer to be deleted
 */
void del_timer(struct timer_root *ptr, struct timer_struct *pts);

/**
 * @brief Update timers' interval in timer rbtree with elapsed time
 *
 * @param [in] elapsed Elapsed timer tick
 */
void update_timer_tree(uint32_t elapsed);

/**
 * @brief Pre allocate oneshot timers
 *
 * This oneshot timer is used in both time delay and pure oneshot task.
 * delay() is none block delay function. When delay() is called,
 * oneshot timer associated with current task is created and added to
 * the timer tree and yield cpu to next task. Once oneshot timer expired
 * (delay time expired), the task is enqueued to runq again and scheduled
 * to work. RT task should not use delay().
 * Max 32 oneshot timer is preallocated and it is wrap-around.
 */
void init_oneshot_timers(void);
#endif

/**
 * @}
 * @}
 * @}
 *
 */

