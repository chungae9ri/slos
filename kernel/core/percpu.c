// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

/**
 * @addtogroup kernel
 * @{
 * @addtogroup kernel_core Core
 * @{
 * @addtogroup kernel_core_proc Process management
 * @{
 *
 * @file
 *
 * @brief Per CPU data container
 *
 */

#include <stdint.h>
#include <percpudef.h>
#include <ktimer.h>
#include <waitq.h>

#if _ENABLE_SMP_
/** Per CPU data offset per CPU */
unsigned long __per_cpu_offset[NR_CPUS] = {0, 0x1000};
/** Per CPU idle task data */
DEFINE_PER_CPU(struct task_struct *, idle_task);
/** Per CPU current task data */
DEFINE_PER_CPU(struct task_struct *, current);
/** Per CPU last task data */
DEFINE_PER_CPU(struct task_struct *, last);
/** Per CPU first task data */
DEFINE_PER_CPU(struct task_struct *, first);
/** Per CPU created task number data */
DEFINE_PER_CPU(uint32_t, task_created_num);
/** Per CPU CFS runq data */
DEFINE_PER_CPU(struct cfs_rq *, runq);
/** Per CPU jiffies data */
DEFINE_PER_CPU(uint32_t, jiffies);
/** Per CPU sched_timer data */
DEFINE_PER_CPU(struct timer_struct *, sched_timer);
/** Per CPU timer root data */
DEFINE_PER_CPU(struct timer_root *, ptroot);
/** Per CPU clock source device data */
DEFINE_PER_CPU(struct clock_source_device *, csd);
/** Per CPU wait queue data */
DEFINE_PER_CPU(struct wait_queue *, wq);
/** Per CPU worker data */
DEFINE_PER_CPU(struct worker *, qworker);
/** Per CPU oneshot_timer data */
DEFINE_PER_CPU(struct timer_struct *, oneshot_timer);
/** Per CPU runq lock data */
DEFINE_PER_CPU(uint32_t, rqlock);
/** Per CPU oneshot_timer index data */
DEFINE_PER_CPU(uint32_t, oneshot_timer_idx);
#else
struct task_struct *idle_task;
struct task_struct *current;
struct task_struct *last;
struct task_struct *first;
/** cpuidle task is not created by forkyi. it is already made from start */
uint32_t task_created_num = 1;
struct cfs_rq *runq;
uint32_t jiffies;
struct timer_struct *sched_timer;
struct timer_root *ptroot;
struct clock_source_device *csd;
struct wait_queue *wq;
struct worker *qworker;
struct timer_struct *oneshot_timer;
uint32_t rqlock;
uint32_t oneshot_timer_idx;

#endif

/**
 * @}
 * @}
 * @}
 *
 */
