#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint-gcc.h>
#include <mem_layout.h>
#include <task.h>
#include <stdlib.h>
#include <timer.h>
#include <wait.h>
#include <defs.h>
#include <ktimer.h>
#include <slmm.h>
#include <xil_printf.h>

#define SVCSPSR 0x13 

uint32_t task_created_num = 1; /* cpuidle task is not created by do_forkyi. it is already made from start */
extern void do_switch_context(struct task_struct *, struct task_struct *);
extern void switch_context_yield(struct task_struct *, struct task_struct *);
extern uint32_t	jiffies;
struct task_struct *current = NULL;
struct task_struct *last = NULL;
struct task_struct *first = NULL;
struct cfs_rq *runq = NULL;
uint32_t show_stat = 0;
extern struct timer_root *ptroot;
volatile uint32_t rqlock = 0;
extern void spin_lock_acquire(volatile uint32_t *pl);
extern void spin_lock_release(volatile uint32_t *pl);
extern char inbyte(void);
extern struct timer_struct *sched_timer;

struct task_struct *idle_task;

void create_cfs_task(char *name, task_entry cfs_task, uint32_t pri)
{
	struct task_struct *temp;

	temp = do_forkyi(name, (task_entry)cfs_task, CFS_TASK);
	set_priority(temp, pri);
	rb_init_node(&temp->se.run_node);
	enqueue_se_to_runq(runq, &temp->se, true);
}

void create_rt_task(char *name, task_entry handler, uint32_t dur)
{
	static uint32_t rt_timer_idx = 0;
	struct task_struct *temp;

	temp = do_forkyi(name, (task_entry)handler, RT_TASK);
	create_rt_timer(temp, dur, rt_timer_idx++, NULL);
}

void rt_worker2(void *arg)
{
	int i, j = 10;
	while (1) {
		/* do some real time work here */
		for (i = 0; i < 10; i++) {
			j++;
		}
		if (show_stat) 	xil_printf("I am rt worker2 \n");
		/* should yield after finish current work */
		yield();
	}
}

void rt_worker1(void *arg)
{
	int i, j = 10;
	while (1) {
		/* do some real time work here */
		for (i = 0; i < 10; i++) {
			j++;
		}
		if (show_stat) 	xil_printf("I am rt worker1\n");
		/* should yield after finish current work */
		yield();
	}
}

extern void enable_interrupt();
extern void disable_interrupt();

void yield(void)
{
	/*uint32_t elapsed;*/
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
	switch_context_yield(temp, current);
}

uint32_t cfs_worker1(void )
{
	xil_printf("I am cfs_worker1....\n");
	while (1) {
		if (show_stat) {
			xil_printf("cfs_worker1 is running....\n");
		}
	}
	return 0;
}

uint32_t cfs_worker2(void)
{
	xil_printf("I am cfs_worker2....\n");
	while (1) {
		if (show_stat) {
			xil_printf("cfs_worker2 running....\n");
		}
	}
	return 0;
}

void init_jiffies(void)
{
	jiffies = 0;
}

/* from lwn.net/Articles/184495 */
void enqueue_se(struct cfs_rq *rq, struct sched_entity *se)
{
	struct rb_node **link = &rq->root.rb_node, *parent=NULL;
	uint64_t value = se->ticks_vruntime;
	int leftmost = 1;

	/* Go to the bottom of the tree */
	while (*link) {
		parent = *link;
		struct sched_entity *entry= rb_entry(parent, struct sched_entity, run_node);

		if (entry->ticks_vruntime > value)
			link = &(*link)->rb_left;
		else if (entry->ticks_vruntime < value) {
			link = &(*link)->rb_right;
			leftmost = 0;
		} else { 
			if (entry->priority > se->priority)
				link = &(*link)->rb_left;
			else {
				link = &(*link)->rb_right;
				leftmost = 0;
			}
		}
	}
	/*
	 * Maintain a cache of leftmost tree entries (it is frequently
	 * used):
	 */
	if (leftmost) 
		rq->rb_leftmost = &se->run_node;

	/* Put the new node there */
	rb_link_node(&se->run_node, parent, link);
	rb_insert_color(&se->run_node, &rq->root);
}

void dequeue_se(struct cfs_rq *rq, struct sched_entity *se)
{
	if (rq->rb_leftmost == (struct rb_node *)&se->run_node) {
		struct rb_node *next_node;

		next_node = rb_next(&se->run_node);
		rq->rb_leftmost = next_node;
	}

	rb_erase(&se->run_node, &rq->root);
}

void update_vruntime_runq(struct cfs_rq *rq, struct sched_entity *se)
{
	struct sched_entity *cur_se, *se_leftmost;
	struct rb_node *cur_rb_node;

	if (!rq->rb_leftmost) return; /* in very first time, the leftmost should be null */

	cur_rb_node = rq->rb_leftmost;
	se_leftmost = container_of(cur_rb_node, struct sched_entity, run_node);
	cur_se = se_leftmost;
	se->ticks_consumed = se_leftmost->ticks_vruntime * rq->priority_sum / se->priority;
	se->ticks_vruntime = se->ticks_consumed * se->priority / rq->priority_sum;

	se->jiffies_consumed = se_leftmost->jiffies_vruntime * rq->priority_sum / se->priority;
	se->jiffies_vruntime = se->jiffies_consumed * se->priority / rq->priority_sum;

	while (cur_se) {
		xil_printf("cur_se : 0x%x\n", cur_se);
		cur_se->ticks_consumed = se_leftmost->ticks_vruntime * rq->priority_sum / cur_se->priority;
		cur_se->ticks_vruntime = cur_se->ticks_consumed * cur_se->priority / rq->priority_sum;

		cur_se->jiffies_consumed = se_leftmost->jiffies_vruntime * rq->priority_sum / cur_se->priority;
		cur_se->jiffies_vruntime = cur_se->jiffies_consumed * cur_se->priority / rq->priority_sum;

		cur_rb_node = rb_next(&cur_se->run_node);
		if (cur_rb_node)
			cur_se = container_of(cur_rb_node, struct sched_entity, run_node);
		else cur_se = NULL;
	}
}

void enqueue_se_to_runq(struct cfs_rq *rq, struct sched_entity *se, bool update)
{
	/*struct sched_entity *se_leftmost;*/
	struct task_struct *tp;

	if (update) {
		tp = container_of(se, struct task_struct, se);
		rq->priority_sum += se->priority;
		remove_from_wq(tp);
		tp->state = TASK_RUNNING;
		spin_lock_acquire(&rqlock);
		update_vruntime_runq(rq, se);
		spin_lock_release(&rqlock);

	}

	spin_lock_acquire(&rqlock);
	enqueue_se(rq, se);
	spin_lock_release(&rqlock);
}

void dequeue_se_to_exit(struct cfs_rq *rq, struct sched_entity *se)
{
	struct task_struct *tp;
	tp = container_of(se, struct task_struct, se);
	if (tp->state == TASK_RUNNING || tp->state == TASK_STOP_RUNNING){
		rq->priority_sum -= se->priority;
		spin_lock_acquire(&rqlock);
		update_vruntime_runq(rq, se);
		dequeue_se(rq, se);
		spin_lock_release(&rqlock);
		tp->state = TASK_STOP;
	} else if (tp->state == TASK_WAITING) {
		spin_lock_acquire(&rqlock);
		remove_from_wq(tp);
		spin_lock_release(&rqlock);
		tp->state = TASK_STOP;
	}
}

void dequeue_se_to_wq(struct cfs_rq *rq, struct sched_entity *se, bool update)
{
	struct task_struct *tp;

	if (update) {
		tp = container_of(se, struct task_struct, se);
		rq->priority_sum -= se->priority;
		add_to_wq(tp);
		tp->state = TASK_WAITING;

		spin_lock_acquire(&rqlock);
		update_vruntime_runq(rq, se);
		spin_lock_release(&rqlock);
	}

	spin_lock_acquire(&rqlock);
	dequeue_se(rq, se);
	spin_lock_release(&rqlock);
}

void set_priority(struct task_struct *pt, uint32_t pri)
{
	pt->se.priority = pri;
}

#include <inttypes.h>
void print_task_stat(void)
{
	struct task_struct *next;
	struct list_head *next_lh;
	int i, idx=0, num=0;
	char buff[256];

	for (i = 0; i < 256; i++) buff[i] = '\0';
	if (current->type == CFS_TASK) {
		num = sprintf(buff,"\ntask:%s\n",current->name);
		idx += num;
		num = sprintf(&buff[idx], "ticks_vruntime:%lu\n",current->se.jiffies_vruntime);
		idx += num;
		num = sprintf(&buff[idx], "ticks_consumed:%lu\n",current->se.jiffies_consumed);
	} 
	xil_printf("%s\n", buff);

	next = current;
	for (;;)  {
		next_lh = next->task.next;
		next = (struct task_struct *)to_task_from_listhead(next_lh);
		if (next == current || !next)
			break;

		num=0; 
		idx=0;
		for (i = 0; i < 256; i++) buff[i] = '\0';

		if (next->type == CFS_TASK) {
			num = sprintf(&buff[idx],"task:%s\n",next->name);
			idx += num;
			num = sprintf(&buff[idx], "vruntime:%lu\n",next->se.jiffies_vruntime);
			idx += num;
			num = sprintf(&buff[idx], "ticks_consumed:%lu\n",next->se.jiffies_consumed);
		} 
		xil_printf("%s\n", buff);
	}
}

#define CMD_LEN		32	
void shell(void)
{
	char byte;
	char cmdline[CMD_LEN];
	int i;

	show_stat = 0;

	xil_printf("I am shell\n");

	while (1) {
		i = 0;
		xil_printf("shell > ");
		do {
			byte = inbyte();
			outbyte(byte);
			cmdline[i++] = byte;

		} while(byte != '\n' && byte != '\r' && i < CMD_LEN);
		cmdline[--i] = '\0';

		xil_printf("\n");
		if (!strcmp(cmdline, "show taskstat")) {
			print_task_stat();
		} else if (!strcmp(cmdline, "show whoami")) {
			show_stat = 1;
		} else if (!strcmp(cmdline, "hide whoami")) {
			show_stat = 0;
		} else if (!strcmp(cmdline, "add cfs task")){
			xil_printf("add cfs task \n");
		} else if (!strcmp(cmdline, "add rt task")){
			xil_printf("add rt task \n");
		} else if (!strcmp(cmdline, "dequeue cfs task")) {
			xil_printf("dequeue cfs task\n");
		} else if (!strcmp(cmdline, "enqueue cfs task")) {
			xil_printf("enqueue cfs task\n");
		} else if (!strcmp(cmdline, "help")) {
			xil_printf("show taskstat, show whoami, hide whoami\n");
		} else {
			xil_printf("I don't know.... ^^;\n");
		}
	}
}

void init_shell(void)
{
	struct task_struct *temp;

	temp = do_forkyi("shell", (task_entry)shell, CFS_TASK);
	set_priority(temp, 2);
	rb_init_node(&temp->se.run_node);
	enqueue_se_to_runq(runq, &temp->se, true);
}

void init_rq(void)
{
	runq = (struct cfs_rq *)kmalloc(sizeof(struct cfs_rq));
	runq->root = RB_ROOT;
	runq->priority_sum = 0;
	runq->rb_leftmost = &idle_task->se.run_node;
	
	rb_init_node(&idle_task->se.run_node);
	enqueue_se_to_runq(runq, &idle_task->se, true);
}

void init_idletask(void)
{
	struct task_struct *pt = (struct task_struct *)kmalloc(sizeof(struct task_struct));
	strcpy(pt->name, "idle task");
	pt->task.next = NULL;
	pt->task.prev = NULL;
	pt->yield_task = NULL;
	pt->se.ticks_vruntime = 0LL;
	pt->se.ticks_consumed = 0LL;
	pt->se.jiffies_vruntime = 0L;
	pt->se.jiffies_consumed = 0L;
	pt->type = CFS_TASK;
	pt->missed_cnt = 0;
	set_priority(pt, 16);
	idle_task = current = first = last = pt;
}

void init_cfs_scheduler(void)
{
	set_ticks_per_sec(get_timer_freq());
	create_sched_timer(NULL, 10, 0, NULL);
	init_jiffies();
}

void init_cfs_workers(void)
{
	create_cfs_task("cfs_worker1", cfs_worker1, 8);
	create_cfs_task("cfs_worker2", cfs_worker2, 2);
}

struct task_struct *do_forkyi(char *name, task_entry fn, TASKTYPE type)
{
	static uint32_t pid = 0;
	struct task_struct *pt;
	if (task_created_num == MAX_TASK) return NULL;

	pt = (struct task_struct *)kmalloc(sizeof(struct task_struct));

	strcpy(pt->name,name);
	pt->pid = pid++;
	pt->entry = fn;
	pt->type = type;
	pt->missed_cnt = 0;
	pt->se.ticks_vruntime = 0LL;
	pt->se.ticks_consumed = 0LL;
	pt->se.jiffies_vruntime = 0L;
	pt->se.jiffies_consumed = 0L;
	pt->yield_task = NULL;
	pt->ct.sp = (uint32_t)(SVC_STACK_BASE - TASK_STACK_GAP * ++task_created_num);
	pt->ct.lr = (uint32_t)pt->entry;
	pt->ct.pc = (uint32_t)pt->entry;
	pt->ct.spsr = SVCSPSR;

	/* get the last task from task list and add this task to the end of the task list*/
	last->task.next = &(pt->task);
	pt->task.prev = &(last->task);
	pt->task.next = &(first->task);
	first->task.prev = &(pt->task);
	last = pt;

	return pt;
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

void update_se(uint32_t elapsed)
{
	struct sched_entity *next_se, *cur_se;
	struct rb_node *next_node;

/* updating vruntime of current is moved to timer_irq routine */
#if 0
	current->se.ticks_consumed += elapsed;
	current->se.ticks_vruntime = (current->se.ticks_consumed) * (current->se.priority)/runq->priority_sum;

	if (&current->se.run_node == runq->rb_leftmost) {
		next_node = rb_next(&current->se.run_node);
		/* if there is only one task in runq, next_node should be NULL */
		if (!next_node) return;
	} else {
		next_node = runq->rb_leftmost;
	}
	se = rb_entry(next_node, struct sched_entity, run_node);
	if (se->ticks_vruntime < current->se.ticks_vruntime) {
		dequeue_se(runq, &current->se);
		enqueue_se(runq, &current->se);
	}
#else
	cur_se = container_of(runq->rb_leftmost, struct sched_entity, run_node);
	next_node = rb_next(runq->rb_leftmost);
	next_se = container_of(next_node, struct sched_entity, run_node);
	if (next_se->ticks_vruntime < cur_se->ticks_vruntime) {
		dequeue_se(runq, cur_se);
		enqueue_se(runq, cur_se);
	}
#endif
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
		current->se.ticks_consumed += elapsed;
		current->se.ticks_vruntime = (current->se.ticks_consumed) *
			(current->se.priority) / runq->priority_sum;
		current->se.jiffies_vruntime = (current->se.jiffies_consumed) *
			(current->se.priority) / runq->priority_sum;
	} else if (current->type == RT_TASK) {
		current->se.ticks_consumed += elapsed;
	}
}
