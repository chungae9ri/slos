#include <stdint-gcc.h>
#include <percpudef.h>
#include <ktimer.h>
#include <waitq.h>
#if _ENABLE_SMP_
unsigned long __per_cpu_offset[NR_CPUS] = {0, 0x1000}; 

DEFINE_PER_CPU(struct task_struct*, idle_task);
DEFINE_PER_CPU(struct task_struct*, current);
DEFINE_PER_CPU(struct task_struct*, last);
DEFINE_PER_CPU(struct task_struct*, first);
DEFINE_PER_CPU(uint32_t, task_created_num);
DEFINE_PER_CPU(struct cfs_rq*, runq);
DEFINE_PER_CPU(uint32_t, jiffies);
DEFINE_PER_CPU(struct timer_struct *, sched_timer);
DEFINE_PER_CPU(struct timer_root *, ptroot);
DEFINE_PER_CPU(struct clock_source_device*, csd);
DEFINE_PER_CPU(struct wait_queue*, wq);
#else
struct task_struct *idle_task;
struct task_struct *current = NULL;
struct task_struct *last = NULL;
struct task_struct *first = NULL;
uint32_t task_created_num = 1; /* cpuidle task is not created by forkyi. it is already made from start */
struct cfs_rq *runq = NULL;
uint32_t jiffies;
struct timer_struct *sched_timer = NULL;
struct timer_root *ptroot = NULL;
struct clock_source_device *csd;
struct wait_queue *wq;
#endif
