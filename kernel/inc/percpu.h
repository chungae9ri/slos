#ifndef _PER_CPU_H_
#define _PER_CPU_H_

#include <percpudef.h>
#if _ENABLE_SMP_
/* per_cpu_xxx storage */
extern DEFINE_PER_CPU(struct task_struct*, idle_task);
extern DEFINE_PER_CPU(struct task_struct*, current);
extern DEFINE_PER_CPU(struct task_struct*, last);
extern DEFINE_PER_CPU(struct task_struct*, first);
extern DEFINE_PER_CPU(uint32_t, task_created_num);
extern DEFINE_PER_CPU(struct cfs_rq*, runq);
extern DEFINE_PER_CPU(uint32_t, jiffies);
extern DEFINE_PER_CPU(struct timer_struct *, sched_timer);
extern DEFINE_PER_CPU(struct timer_root *, ptroot);
extern DEFINE_PER_CPU(struct clock_source_device *, csd);
extern DEFINE_PER_CPU(struct wait_queue*, wq);
extern DEFINE_PER_CPU(uint32_t, tc);
#else /* ! _ENABLE_SMP_*/
extern struct task_struct *idle_task;
extern struct task_struct *current;
extern struct task_struct *last;
extern struct task_struct *first;
extern uint32_t task_created_num; /* cpuidle task is not created by forkyi. it is already made from start */
extern struct cfs_rq *runq;
extern uint32_t jiffies;
extern struct timer_struct *sched_timer;
extern struct timer_root *ptroot;
extern struct clock_source_device *csd;
extern struct wait_queue *wq;
extern uint32_t tc;
#endif /* ! _ENABLE_SMP_*/
#endif