// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

#include <stdint.h>
#include <percpudef.h>
#include <ktimer.h>
#include <waitq.h>

#if _ENABLE_SMP_
unsigned long __per_cpu_offset[NR_CPUS] = {0, 0x1000};

DEFINE_PER_CPU(struct task_struct *, idle_task);
DEFINE_PER_CPU(struct task_struct *, current);
DEFINE_PER_CPU(struct task_struct *, last);
DEFINE_PER_CPU(struct task_struct *, first);
DEFINE_PER_CPU(uint32_t, task_created_num);
DEFINE_PER_CPU(struct cfs_rq *, runq);
DEFINE_PER_CPU(uint32_t, jiffies);
DEFINE_PER_CPU(struct timer_struct *, sched_timer);
DEFINE_PER_CPU(struct timer_root *, ptroot);
DEFINE_PER_CPU(struct clock_source_device *, csd);
DEFINE_PER_CPU(struct wait_queue *, wq);
DEFINE_PER_CPU(struct worker *, qworker);
DEFINE_PER_CPU(struct timer_struct *, oneshot_timer);
DEFINE_PER_CPU(uint32_t, rqlock);
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
