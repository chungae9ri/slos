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

struct task_struct *current = NULL;
struct task_struct *last = NULL;
struct task_struct *first = NULL;
uint32_t task_created_num = 1; /* cpuidle task is not created by forkyi. it is already made from start */
extern struct task_struct *idle_task;
extern struct cfs_rq *runq;

void init_cfs_scheduler(void)
{
	set_ticks_per_sec(get_timer_freq());
	create_sched_timer(current, 10, 0, NULL);
	init_jiffies();
}

void init_jiffies(void)
{
	jiffies = 0;
}

void set_priority(struct task_struct *pt, uint32_t pri)
{
	pt->se.priority = pri;
}

#include <inttypes.h>
void print_task_stat(void)
{
	char buff[256];
	int i, num = 0;
	struct task_struct *pcur = NULL;
	struct list_head *next_lh = NULL;

	next_lh = &first->task;
	pcur = (struct task_struct *)to_task_from_listhead(next_lh);
	/*for (i = 0; i < runq->cfs_task_num; i++) {*/
	do {
		if (!pcur) break;

		num = 0;
		for (i = 0; i < 256; i++) buff[i] = '\0';
		if (pcur->type == CFS_TASK) {
			num += sprintf(&buff[num],"cfs task:%s\n", pcur->name);
			num += sprintf(&buff[num], "pid: %lu\n", pcur->pid);
			num += sprintf(&buff[num], "state: %lu\n", pcur->state);
			num += sprintf(&buff[num], "priority: %lu\n", pcur->se.priority);
			num += sprintf(&buff[num], "jiffies_vruntime: %lu\n", pcur->se.jiffies_vruntime);
			num = sprintf(&buff[num], "jiffies_consumed: %lu\n", pcur->se.jiffies_consumed);
			xil_printf("%s\n", buff);
		} else if (pcur->type == RT_TASK) {
			num += sprintf(&buff[num],"rt task:%s\n", pcur->name);
			num += sprintf(&buff[num], "pid: %lu\n", pcur->pid);
			num += sprintf(&buff[num], "state: %lu\n", pcur->state);
			num += sprintf(&buff[num], "time interval: %lu msec\n", pcur->timeinterval);
			num += sprintf(&buff[num], "deadline %lu times missed\n", pcur->missed_cnt);
			xil_printf("%s\n", buff);
		}

		next_lh = next_lh->next;
		pcur = (struct task_struct *)to_task_from_listhead(next_lh);
	} while (pcur != first);
}

void schedule(void)
{
	struct task_struct *next;
	struct sched_entity *se;

	/* Don't check task type since CSF task can interrupt RT task!! */
	se = rb_entry(runq->rb_leftmost, struct sched_entity, run_node);
	next = (struct task_struct *)to_task_from_se(se);

	/* you should not print message in interrupt context. 
	   print message try to acquire spin lock but if it fails, it spins forever 
	   because interrupt is disabled.
	 */
	/*if (show_stat && next->pfwhoami) next->pfwhoami();*/

	if (current==next) return;

	switch_context(current, next);
	next->yield_task = current;
	current = next;
}

struct task_struct *forkyi(char *name, task_entry fn, TASKTYPE type)
{
	/* cpuidle is pid 0 */
	uint32_t lr = 0;
	static uint32_t pid = 1;
	struct task_struct *pt;
	if (task_created_num == MAX_TASK) return NULL;

	pt = (struct task_struct *)kmalloc(sizeof(struct task_struct));

	pt->pid = pid++;
	pt->entry = fn;
	pt->type = type;
	pt->missed_cnt = 0;
	strcpy(pt->name,name);
	/*pt->se.ticks_vruntime = 0LL;*/
	pt->se.ticks_consumed = 0LL;
	pt->se.jiffies_vruntime = 0L;
	pt->se.jiffies_consumed = 0L;
	pt->yield_task = NULL;
	pt->preempted = 0;
	pt->done = 1;
	pt->ct.sp = (uint32_t)(SVC_STACK_BASE - TASK_STACK_GAP * ++task_created_num);
	asm ("mov %0, r14" : "+r" (lr) : : );
	pt->ct.lr = (uint32_t)lr;
	pt->ct.pc = (uint32_t)pt->entry;

	/* get the last task from task list and add this task to the end of the task list*/
	last->task.next = &(pt->task);
	pt->task.prev = &(last->task);
	pt->task.next = &(first->task);
	first->task.prev = &(pt->task);
	last = pt;

	return pt;
}

extern void disable_interrupt(void);
void yield(void)
{
	uint32_t lr = 0;
	struct task_struct *temp;

	disable_interrupt();
	/* need to update ticks_consumed of RT task
	   but data abort exception happens. Fix me
	 */
#if 0
	elapsed = get_elapsedtime();
	update_timer_tree(elapsed);
	current->se.ticks_consumed += elapsed;
#endif
	temp = current;
	if (current->yield_task->state == TASK_RUNNING)
		current = current->yield_task;
	else current = idle_task;
	/* keep current lr and save it to task's ctx */
	asm ("mov %0, r14" : "+r" (lr) : : );
	switch_context_yield(temp, current, lr);
}

void switch_context(struct task_struct *prev, struct task_struct *next)
{
	do_switch_context(prev, next);
}

void update_current(uint32_t elapsed)
{
	if (current->type == CFS_TASK) {
		jiffies++;
		current->se.jiffies_consumed++;
		current->se.ticks_consumed += (uint64_t) elapsed;
		/*current->se.ticks_vruntime = (current->se.ticks_consumed) *
			(current->se.priority) / runq->priority_sum;
			*/
		current->se.jiffies_vruntime = (current->se.jiffies_consumed) *
			(current->se.priority) / runq->priority_sum;
	} else if (current->type == RT_TASK) {
		current->se.ticks_consumed += (uint64_t)elapsed;
	}
}

