/*
  kernel/core/sched.c process management
  (C) 2018 Kwangdo Yi <kwangdo.yi@gmail.com>
 
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, see <http://www.gnu.org/licenses/>
*/
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <timer.h>
#include <ktimer.h>
#include <runq.h>
#include <xil_printf.h>
#include <percpu.h>

extern uint32_t smp_processor_id(void);

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

	if (cpuid == 0)
		set_ticks_per_sec(get_timer_freq());

	create_sched_timer(this_current, 10, NULL);
	init_jiffies();
}

void init_jiffies(void)
{
#if _ENABLE_SMP_
	__get_cpu_var(jiffies) = 0;
#else
	jiffies = 0;
#endif
}

#include <inttypes.h>
void print_task_stat(void *arg)
{
	char buff[256];
	int i, num = 0;
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
	xil_printf("**** cpu:%d taskstat ****\n", cpuid);
	next_lh = &this_first->task;
	pcur = (struct task_struct *)to_task_from_listhead(next_lh);
	/*for (i = 0; i < runq->cfs_task_num; i++) {*/
	do {
		if (!pcur) break;

		num = 0;
		for (i = 0; i < 256; i++) buff[i] = '\0';
		if (pcur->type == CFS_TASK) {
			num += sprintf(&buff[num],"cfs task:%s\n", pcur->name);
			num += sprintf(&buff[num], "pid: %lu\n", pcur->pid);
			num += sprintf(&buff[num], "state: %lu\n", (uint32_t)pcur->state);
			num += sprintf(&buff[num], "priority: %lu\n", pcur->se.priority);
			num += sprintf(&buff[num], "jiffies_vruntime: %lu\n", pcur->se.jiffies_vruntime);
			num = sprintf(&buff[num], "jiffies_consumed: %lu\n", pcur->se.jiffies_consumed);
			xil_printf("%s\n", buff);
		} else if (pcur->type == RT_TASK) {
			num += sprintf(&buff[num],"rt task:%s\n", pcur->name);
			num += sprintf(&buff[num], "pid: %lu\n", pcur->pid);
			num += sprintf(&buff[num], "state: %lu\n", (uint32_t)pcur->state);
			num += sprintf(&buff[num], "time interval: %lu msec\n", pcur->timeinterval);
			num += sprintf(&buff[num], "deadline %lu times missed\n", pcur->missed_cnt);
			xil_printf("%s\n", buff);
		}

		next_lh = next_lh->next;
		pcur = (struct task_struct *)to_task_from_listhead(next_lh);
	} while (pcur != this_first);
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

	/* you should not print message in interrupt context. 
	   print message try to acquire spin lock but if it fails, it spins forever 
	   because interrupt is disabled.
	 */
	/*if (show_stat && next->pfwhoami) next->pfwhoami();*/

#if _ENABLE_SMP_
	this_current = __get_cpu_var(current);
#else
	this_current = current;
#endif
	if (this_current == next) 
		return;

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
		} else {
			pri_div_shift = i;
		}
	}

#if _ENABLE_SMP_
	this_first = __get_cpu_var(first);
	this_last = __get_cpu_var(last);
	pthis_task_created_num =(uint32_t *) __get_cpu_var_addr(task_created_num);
	this_current = __get_cpu_var(current);
#else
	this_first = first;
	this_last = last;
	pthis_task_created_num = &task_created_num;
	this_current = current;
#endif

	if (*pthis_task_created_num == MAX_TASK) 
		return NULL;

	pt = (struct task_struct *)kmalloc(sizeof(struct task_struct));

	pt->pid = pid++;
	pt->entry = fn;
	pt->type = type;
	pt->missed_cnt = 0;
	strcpy(pt->name,name);
	if (type == CFS_TASK) {
		pt->se.pri_div_shift = pri_div_shift;
		pt->se.priority = pri;
	}
	/*pt->se.ticks_vruntime = 0LL;*/
	pt->se.ticks_consumed = 0LL;
	pt->se.jiffies_vruntime = 0L;
	pt->se.jiffies_consumed = 0L;
	pt->yield_task = this_current;
	pt->preempted = 0;
	pt->done = 1;
	(*pthis_task_created_num)++;
	cpuid = smp_processor_id();
	if (cpuid == 0)
		pt->ct.sp = (uint32_t)(SVC_STACK_BASE - TASK_STACK_GAP * (*pthis_task_created_num));
	else 
		pt->ct.sp = (uint32_t)(SEC_SVC_STACK_BASE - TASK_STACK_GAP * (*pthis_task_created_num));

	asm ("mov %0, r14" : "+r" (lr) : : );
	pt->ct.lr = (uint32_t)lr;
	pt->ct.pc = (uint32_t)pt->entry;

	/* get the last task from task list and add this task to the end of the task list*/
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
/* yield() isn't fully thread-safe. yield() is used
 * from RT task and msleep() and ramdomly crashed
 * when both tasks are running. Fix me 
 */
void yield(void)
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

	/* only cpuidle task has NULL for yield_task.
	 * This is true because cpuidle doesn't yield, rather
	 * it enters arch-dependent power collapse routines.
	 */
	if (!this_current->yield_task)
		return;

	disable_interrupt();
	/* need to update ticks_consumed of RT task
	 * but data abort exception happens. Fix me
	 */
#if 0
	elapsed = get_elapsedtime();
	update_timer_tree(elapsed);
	current->se.ticks_consumed += elapsed;
#endif

	temp = this_current;

#if _ENABLE_SMP_
	if (this_current->yield_task->state == TASK_RUNNING)
		__get_cpu_var(current) = this_current->yield_task;
	else 
		__get_cpu_var(current) = this_idle_task;

	this_current = __get_cpu_var(current);
	switch_context_yield(temp, this_current);
#else
	if (this_current->yield_task->state == TASK_RUNNING)
		current = this_current->yield_task;
	else 
		current = this_idle_task;

	switch_context_yield(temp, current);
#endif
}

void switch_context(struct task_struct *prev, struct task_struct *next)
{
	do_switch_context(prev, next);
}

void update_current(uint32_t elapsed)
{
	uint32_t mul;
	struct task_struct *this_current = NULL;
	struct cfs_rq *this_runq = NULL;
#if _ENABLE_SMP_
	this_current = __get_cpu_var(current);
	this_runq = __get_cpu_var(runq);
#else
	this_current = current;
	this_runq = runq;
#endif

	if (this_current->type == CFS_TASK) {
#if _ENABLE_SMP_
		__get_cpu_var(jiffies) += 1;
#else
		jiffies++;
#endif
		this_current->se.jiffies_consumed++;
		this_current->se.ticks_consumed += (uint64_t) elapsed;
		/*current->se.ticks_vruntime = (current->se.ticks_consumed) *
			(current->se.priority) / runq->priority_sum;
			*/
#ifdef FREESTANDING
		mul = (this_current->se.jiffies_consumed) * (this_current->se.priority);
		div_sf(&mul, &(this_runq->priority_sum), &(this_current->se.jiffies_vruntime));
#else
		this_current->se.jiffies_vruntime = (this_current->se.jiffies_consumed) *
			(this_current->se.priority);
		/*
		this_current->se.jiffies_vruntime = (this_current->se.jiffies_consumed) *
			(this_current->se.priority) / this_runq->priority_sum;
		*/
#endif
	} else if (this_current->type == RT_TASK) {
		this_current->se.ticks_consumed += (uint64_t)elapsed;
	}
}
