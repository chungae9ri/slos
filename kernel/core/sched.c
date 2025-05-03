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
 * @brief Functions for kernel process schedule, context switch, yield, fork new task
 *
 */

#include <sched.h>
#include <timer.h>
#include <ktimer.h>
#include <runq.h>
#include <printk.h>
#include <percpu.h>
#include <ops.h>

/** CFS priority number */
#define CFS_PRI_NUM (5u)
/** Mas task number supported */
#define MAX_TASK ((SVC_STACK_BASE - SYS_STACK_BASE) / (TASK_STACK_GAP))

static void init_jiffies(void)
{
#if _ENABLE_SMP_
	__get_cpu_var(jiffies) = 0;
#else
	jiffies = 0;
#endif
}

void init_cfs_scheduler(void)
{
	uint32_t cpuid = 0;
	struct task_struct *this_current = NULL;
#if _ENABLE_SMP_
	this_current = __get_cpu_var(current);
#else
	this_current = current;
#endif
	cpuid = smp_processor_id();

	if (cpuid == 0) {
		set_ticks_per_sec(get_timer_freq());
	}

	create_sched_timer(this_current, 10, NULL);
	init_jiffies();
}

int32_t print_task_stat(void *arg)
{
	uint32_t cpuid;
	struct task_struct *pcur = NULL;
	struct list_head *next_lh = NULL;
	struct task_struct *this_first = NULL;

#if _ENABLE_SMP_
	this_first = __get_cpu_var(first);
#else
	this_first = first;
#endif
	cpuid = smp_processor_id();
	printk("**** cpu: %d taskstat ****\n", cpuid);
	next_lh = &this_first->task;
	pcur = (struct task_struct *)to_task_from_listhead(next_lh);
	do {
		if (!pcur) {
			break;
		}

		if (pcur->type == CFS_TASK) {
			printk("cfs task:%s\n", pcur->name);
			printk("pid: %d\n", pcur->pid);
			printk("state: %d\n", (uint32_t)pcur->state);
			printk("priority: %d\n", pcur->se.priority);
			printk("jiffies_vruntime: %d\n", pcur->se.jiffies_vruntime);
			printk("jiffies_consumed: %d\n", pcur->se.jiffies_consumed);
		} else if (pcur->type == RT_TASK) {
			printk("rt task:%s\n", pcur->name);
			printk("pid: %d\n", pcur->pid);
			printk("state: %d\n", (uint32_t)pcur->state);
			printk("time interval: %d msec\n", pcur->timeinterval);
			printk("deadline %d times missed\n", pcur->missed_cnt);
		}

		next_lh = next_lh->next;
		pcur = (struct task_struct *)to_task_from_listhead(next_lh);
	} while (pcur != this_first);

	return 0;
}

void schedule(void)
{
	struct task_struct *next = NULL;
	struct sched_entity *se = NULL;
	struct task_struct *this_current = NULL;
	struct cfs_rq *this_runq = NULL;

#if _ENABLE_SMP_
	this_runq = __get_cpu_var(runq);
#else
	this_runq = runq;
#endif

	/* Don't check task type since CSF task can interrupt RT task!! */
	se = rb_entry(this_runq->rb_leftmost, struct sched_entity, run_node);
	next = (struct task_struct *)to_task_from_se(se);

	/* You should not print message in interrupt context.
	 * Print message try to acquire spin lock but if it fails, it spins
	 * forever because interrupt is disabled.
	 */

#if _ENABLE_SMP_
	this_current = __get_cpu_var(current);
#else
	this_current = current;
#endif
	if (this_current == next) {
		return;
	}

	switch_context(this_current, next);
	/*next->yield_task = this_current;*/
#if _ENABLE_SMP_
	__get_cpu_var(current) = next;
#else
	current = next;
#endif
}

struct task_struct *forkyi(char *name, task_entry fn, TASKTYPE type, uint32_t pri)
{
	uint32_t i;
	uint32_t pri_div_shift;
	/* cpuidle is pid 0 */
	uint32_t lr = 0, cpuid;
#if _ENABLE_SMP_
	static uint32_t pid = 2;
#else
	static uint32_t pid = 1;
#endif
	struct task_struct *pt = NULL;
	struct task_struct *this_first = NULL;
	struct task_struct *this_last = NULL;
	struct task_struct *this_current = NULL;
	uint32_t *pthis_task_created_num = NULL;

	/* CFS task priority should be one of 1 2 4 8 16 */
	if (type == CFS_TASK) {
		for (i = 0; i < CFS_PRI_NUM; i++) {
			if (pri == (0x1 << i)) {
				break;
			}
		}

		if (i == CFS_PRI_NUM) {
			return NULL;
		}

		pri_div_shift = i;
	}

#if _ENABLE_SMP_
	this_first = __get_cpu_var(first);
	this_last = __get_cpu_var(last);
	pthis_task_created_num = (uint32_t *)__get_cpu_var_addr(task_created_num);
	this_current = __get_cpu_var(current);
#else
	this_first = first;
	this_last = last;
	pthis_task_created_num = &task_created_num;
	this_current = current;
#endif

	if (*pthis_task_created_num == MAX_TASK) {
		return NULL;
	}

	pt = kmalloc(sizeof(struct task_struct));

	pt->pid = pid++;
	pt->entry = fn;
	pt->type = type;
	pt->missed_cnt = 0;
	i = 0;
	while (name[i] != '\0') {
		pt->name[i] = name[i];
		i++;
	}
	pt->name[i] = '\0';

	if (type == CFS_TASK) {
		pt->se.pri_div_shift = pri_div_shift;
		pt->se.priority = pri;
	}

	pt->se.ticks_consumed = 0LL;
	pt->se.jiffies_vruntime = 0L;
	pt->se.jiffies_consumed = 0L;
	pt->yield_task = this_current;
	pt->preempted = 0;
	pt->done = 1;
	(*pthis_task_created_num)++;
	cpuid = smp_processor_id();
	if (cpuid == 0) {
		pt->ct.sp = (uint32_t)(SVC_STACK_BASE - TASK_STACK_GAP * (*pthis_task_created_num));
	} else {
		pt->ct.sp =
		    (uint32_t)(SEC_SVC_STACK_BASE - TASK_STACK_GAP * (*pthis_task_created_num));
	}

	__asm __volatile("mov %[lr], r14" : [lr] "+r"(lr)::);
	pt->ct.lr = (uint32_t)lr;
	pt->ct.pc = (uint32_t)pt->entry;
	pt->ct.fpscr = 0x0;
	pt->ct.fpexc = 0x40000000;

	/* Get the last task from task list and add this task to the end of the task list */
	this_last->task.next = &(pt->task);
	pt->task.prev = &(this_last->task);
	pt->task.next = &(this_first->task);
	this_first->task.prev = &(pt->task);
#if _ENABLE_SMP_
	__get_cpu_var(last) = pt;
#else
	last = pt;
#endif

	return pt;
}

extern void disable_interrupt(void);
/* yieldyi() isn't fully thread-safe. yieldyi() is used
 * from RT task and mdelay() and ramdomly crashed
 * when both tasks are running. Fix me
 */
void yieldyi(void)
{
	struct task_struct *temp = NULL;
	struct task_struct *this_current = NULL, *this_idle_task = NULL;

#if _ENABLE_SMP_
	this_current = __get_cpu_var(current);
	this_idle_task = __get_cpu_var(idle_task);
#else
	this_current = current;
	this_idle_task = idle_task;
#endif

	/* Only cpuidle task has NULL for yield_task.
	 * This is true because cpuidle doesn't yield, rather
	 * it enters arch-dependent power collapse routines.
	 */
	if (!this_current->yield_task) {
		return;
	}

	disable_interrupt();

	/* Need to update ticks_consumed of RT task
	 * but data abort exception happens. Fix me
	 */
	temp = this_current;

#if _ENABLE_SMP_
	if (this_current->yield_task->state == TASK_RUNNING) {
		__get_cpu_var(current) = this_current->yield_task;
	} else {
		__get_cpu_var(current) = this_idle_task;
	}

	this_current = __get_cpu_var(current);
	switch_context_yieldyi(temp, this_current);
#else
	if (this_current->yield_task->state == TASK_RUNNING) {
		current = this_current->yield_task;
	} else {
		current = this_idle_task;
	}

	switch_context_yieldyi(temp, current);
#endif
}

void switch_context(struct task_struct *prev, struct task_struct *next)
{
	do_switch_context(prev, next);
}

void update_current(uint32_t elapsed)
{
	struct task_struct *this_current = NULL;
#if _ENABLE_SMP_
	this_current = __get_cpu_var(current);
#else
	this_current = current;
#endif

	if (this_current->type == CFS_TASK) {
#if _ENABLE_SMP_
		__get_cpu_var(jiffies) += 1;
#else
		jiffies++;
#endif
		this_current->se.jiffies_consumed++;
		this_current->se.ticks_consumed += (uint64_t)elapsed;
		this_current->se.jiffies_vruntime =
		    (this_current->se.jiffies_consumed) * (this_current->se.priority);
	} else if (this_current->type == RT_TASK) {
		this_current->se.ticks_consumed += (uint64_t)elapsed;
	}
}

/**
 * @}
 * @}
 * @}
 *
 */
