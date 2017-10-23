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

uint32_t task_created_num = 1; /* cpuidle task is not created by forkyi. it is already made from start */
extern void do_switch_context(struct task_struct *, struct task_struct *);
extern void switch_context_yield(struct task_struct *, struct task_struct *);
extern uint32_t	jiffies;
struct task_struct *current = NULL;
struct task_struct *last = NULL;
struct task_struct *first = NULL;
static struct cfs_rq *runq = NULL;
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

	temp = forkyi(name, (task_entry)cfs_task, CFS_TASK);
	set_priority(temp, pri);
	rb_init_node(&temp->se.run_node);
	enqueue_se_to_runq(&temp->se, true);
	runq->cfs_task_num++;
}

void create_rt_task(char *name, task_entry handler, uint32_t dur)
{
	static uint32_t rt_timer_idx = 0;
	struct task_struct *temp;

	temp = forkyi(name, (task_entry)handler, RT_TASK);
	temp->timeinterval = dur;
	temp->state = TASK_RUNNING;
	create_rt_timer(temp, dur, rt_timer_idx++, NULL);
}

void create_oneshot_task(char *name, task_entry handler, uint32_t dur)
{
	static uint32_t oneshot_timer_idx = 0;
	struct task_struct *temp;

	temp = forkyi(name, (task_entry)handler, ONESHOT_TASK);
	temp->timeinterval = dur;
	temp->state = TASK_RUNNING;
	create_oneshot_timer(temp, dur, oneshot_timer_idx++, NULL);
}

uint32_t rt_worker2(void)
{
	int i, j = 0;
	while (1) {
		/* do some real time work here */
		for (i = 0; i < 500; i++) {
			j++;
		}
		j = 0;

		if (show_stat) {
			xil_printf("I am rt worker2 \n");
		}
		/* should yield after finish current work */
		yield();
	}
	return 0;
}

uint32_t rt_worker1(void)
{
	int i, j = 0;
	while (1) {
		/* do some real time work here */
		for (i = 0; i < 1000; i++) {
			j++;
		}
		j = 0;

		if (show_stat) {
			xil_printf("I am rt worker1\n");	
		}
		/* should yield after finish current work */
		yield();
	}
	return 0;
}

uint32_t oneshot_worker(void)
{
	int i, j = 0;
	while (1) {
		/* do some real time work here */
		for (i = 0; i < 1000; i++) {
			j++;
		}
		j = 0;

		xil_printf("I am oneshot_worker\n");
		/* should yield after finish current work */
		yield();
	}

	return 0;
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
void enqueue_se(struct sched_entity *se)
{
	struct rb_node **link = &runq->root.rb_node, *parent=NULL;
	uint64_t value = se->jiffies_vruntime;
	int leftmost = 1;

	/* Go to the bottom of the tree */
	while (*link) {
		parent = *link;
		struct sched_entity *entry= rb_entry(parent, struct sched_entity, run_node);

		if (entry->jiffies_vruntime >= value)
			link = &(*link)->rb_left;
		else if (entry->jiffies_vruntime < value) {
			link = &(*link)->rb_right;
			leftmost = 0;
		} /*else { 
			if (entry->priority > se->priority)
				link = &(*link)->rb_left;
			else {
				link = &(*link)->rb_right;
				leftmost = 0;
			}
		}*/
	}
	/*
	 * Maintain a cache of leftmost tree entries (it is frequently
	 * used):
	 */
	if (leftmost) 
		runq->rb_leftmost = &se->run_node;

	/* Put the new node there */
	rb_link_node(&se->run_node, parent, link);
	rb_insert_color(&se->run_node, &runq->root);
}

void dequeue_se(struct sched_entity *se)
{
	if (runq->rb_leftmost == (struct rb_node *)&se->run_node) {
		struct rb_node *next_node;

		next_node = rb_next(&se->run_node);
		runq->rb_leftmost = next_node;
	}

	rb_erase(&se->run_node, &runq->root);
}

void update_vruntime_runq(struct sched_entity *se)
{
	struct sched_entity *cur_se, *se_leftmost;
	struct rb_node *cur_rb_node;
	/* in very first time, the leftmost should be null */
	if (!runq->rb_leftmost) 
		return; 

	cur_rb_node = runq->rb_leftmost;
	se_leftmost = container_of(cur_rb_node, struct sched_entity, run_node);
	cur_se = se_leftmost;

	se->jiffies_consumed = se_leftmost->jiffies_vruntime * runq->priority_sum / se->priority;
	se->jiffies_vruntime = se->jiffies_consumed * se->priority / runq->priority_sum;

	while (cur_se) {
		cur_se->jiffies_consumed = se_leftmost->jiffies_vruntime * runq->priority_sum / cur_se->priority;
		cur_se->jiffies_vruntime = cur_se->jiffies_consumed * cur_se->priority / runq->priority_sum;

		cur_rb_node = rb_next(&cur_se->run_node);
		if (cur_rb_node)
			cur_se = container_of(cur_rb_node, struct sched_entity, run_node);
		else cur_se = NULL;
	}
}

void enqueue_se_to_runq(struct sched_entity *se, bool update)
{
	/*struct sched_entity *se_leftmost;*/
	struct task_struct *tp;

	if (update) {
		tp = container_of(se, struct task_struct, se);
		runq->priority_sum += se->priority;
		remove_from_wq(tp);
		tp->state = TASK_RUNNING;
		spin_lock_acquire(&rqlock);
		update_vruntime_runq(se);
		spin_lock_release(&rqlock);

	}

	spin_lock_acquire(&rqlock);
	enqueue_se(se);
	spin_lock_release(&rqlock);
}

void dequeue_se_to_exit(struct sched_entity *se)
{
	struct task_struct *tp;
	tp = container_of(se, struct task_struct, se);
	if (tp->state == TASK_RUNNING || tp->state == TASK_STOP_RUNNING){
		runq->priority_sum -= se->priority;
		spin_lock_acquire(&rqlock);
		update_vruntime_runq(se);
		dequeue_se(se);
		spin_lock_release(&rqlock);
		tp->state = TASK_STOP;
	} else if (tp->state == TASK_WAITING) {
		spin_lock_acquire(&rqlock);
		remove_from_wq(tp);
		spin_lock_release(&rqlock);
		tp->state = TASK_STOP;
	}
}

void dequeue_se_to_wq(struct sched_entity *se, bool update)
{
	struct task_struct *tp;

	if (update) {
		tp = container_of(se, struct task_struct, se);
		runq->priority_sum -= se->priority;
		add_to_wq(tp);
		tp->state = TASK_WAITING;

		spin_lock_acquire(&rqlock);
		update_vruntime_runq(se);
		spin_lock_release(&rqlock);
	}

	spin_lock_acquire(&rqlock);
	dequeue_se(se);
	spin_lock_release(&rqlock);
}

void set_priority(struct task_struct *pt, uint32_t pri)
{
	pt->se.priority = pri;
}

void create_cfs_workers(void)
{
	create_cfs_task("cfs_worker1", cfs_worker1, 8);
	create_cfs_task("cfs_worker2", cfs_worker2, 4);
}

void create_rt_workers(void)
{
	create_rt_task("rt_worker1", rt_worker1, 20);
	create_rt_task("rt_worker2", rt_worker2, 25);
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

#define CMD_LEN		32	

void shell(void)
{
	char byte;
	char cmdline[CMD_LEN];
	int i, j, pid;
	struct task_struct *pwait_task;
	struct list_head *next_lh = NULL;

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
		if (cmdline[0] == '\0' || !strcmp(cmdline, "help")) {
			xil_printf("taskstat, whoami, hide whoami\n");
			xil_printf("cfs task, rt task, oneshot task\n");
			xil_printf("sleep, run \n");
		} else if (!strcmp(cmdline, "taskstat")) {
			print_task_stat();
		} else if (!strcmp(cmdline, "whoami")) {
			show_stat = 1;
		} else if (!strcmp(cmdline, "hide whoami")) {
			show_stat = 0;
		} else if (!strcmp(cmdline, "cfs task")) {
			xil_printf("add cfs task \n");
			create_cfs_workers();
		} else if (!strcmp(cmdline, "rt task")) {
			xil_printf("add rt task \n");
			create_rt_workers();
		} else if (!strcmp(cmdline, "oneshot task")) {
			xil_printf("add oneshottask \n");
			create_oneshot_task("oneshot_task", oneshot_worker, 1000);
		} else if (!strcmp(cmdline, "sleep")) {
			xil_printf("input task pid: ");
			byte = inbyte();
			outbyte(byte);
			outbyte('\n');
			pid = byte - '0';

			next_lh = &first->task;
			for (j = 0; j < runq->cfs_task_num; j++) {
				pwait_task = (struct task_struct *)to_task_from_listhead(next_lh);
				if (pwait_task->pid == pid) break;
				next_lh = next_lh->next;
			}
			if (j < runq->cfs_task_num && pwait_task->state == TASK_RUNNING) {
				dequeue_se_to_wq(&pwait_task->se, true);
			} else {
				xil_printf("task %d is not in runq\n", pid);
			}

		} else if (!strcmp(cmdline, "run")) {
			xil_printf("input task pid: ");
			byte = inbyte();
			outbyte(byte);
			outbyte('\n');
			pid = byte - '0';

			next_lh = &first->task;
			for (j = 0; j < runq->cfs_task_num; j++) {
				pwait_task = (struct task_struct *)to_task_from_listhead(next_lh);
				if (pwait_task->pid == pid) break;
				next_lh = next_lh->next;
			}
			if (j < runq->cfs_task_num && pwait_task->state == TASK_WAITING) {
				enqueue_se_to_runq(&pwait_task->se, true);
			} else {
				xil_printf("task %d is not in runq\n", pid);
			}
		} else {
			xil_printf("I don't know.... ^^;\n");
		}
	}
}

void init_shell(void)
{
	struct task_struct *temp;

	temp = forkyi("shell", (task_entry)shell, CFS_TASK);
	set_priority(temp, 2);
	rb_init_node(&temp->se.run_node);
	enqueue_se_to_runq(&temp->se, true);
	runq->cfs_task_num++;
}

void init_rq(void)
{
	runq = (struct cfs_rq *)kmalloc(sizeof(struct cfs_rq));
	runq->root = RB_ROOT;
	runq->priority_sum = 0;
	runq->rb_leftmost = &idle_task->se.run_node;
	runq->cfs_task_num = 0;
	
	rb_init_node(&idle_task->se.run_node);
	enqueue_se_to_runq(&idle_task->se, true);
	runq->cfs_task_num++;
}

void init_idletask(void)
{
	struct task_struct *pt = (struct task_struct *)kmalloc(sizeof(struct task_struct));
	strcpy(pt->name, "idle task");
	pt->task.next = NULL;
	pt->task.prev = NULL;
	pt->yield_task = NULL;
	/*pt->se.ticks_vruntime = 0LL;*/
	pt->se.ticks_consumed = 0LL;
	pt->se.jiffies_vruntime = 0L;
	pt->se.jiffies_consumed = 0L;
	pt->type = CFS_TASK;
	pt->missed_cnt = 0;
	pt->pid = 0;
	set_priority(pt, 16);
	idle_task = current = first = last = pt;
}

void init_cfs_scheduler(void)
{
	set_ticks_per_sec(get_timer_freq());
	create_sched_timer(NULL, 10, 0, NULL);
	init_jiffies();
}

struct task_struct *forkyi(char *name, task_entry fn, TASKTYPE type)
{
	/* cpuidle is pid 0 */
	uint32_t lr = 0;
	static uint32_t pid = 1;
	struct task_struct *pt;
	if (task_created_num == MAX_TASK) return NULL;

	pt = (struct task_struct *)kmalloc(sizeof(struct task_struct));

	strcpy(pt->name,name);
	pt->pid = pid++;
	pt->entry = fn;
	pt->type = type;
	pt->missed_cnt = 0;
	/*pt->se.ticks_vruntime = 0LL;*/
	pt->se.ticks_consumed = 0LL;
	pt->se.jiffies_vruntime = 0L;
	pt->se.jiffies_consumed = 0L;
	pt->yield_task = NULL;
	pt->ct.sp = (uint32_t)(SVC_STACK_BASE - TASK_STACK_GAP * ++task_created_num);
	asm ("mov %0, r14" : "+r" (lr) : : );
	/*pt->ct.lr = (uint32_t)pt->entry;*/
	pt->ct.lr = (uint32_t)lr;
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
		dequeue_se(&current->se);
		enqueue_se(runq, &current->se);
	}
#else
	cur_se = container_of(runq->rb_leftmost, struct sched_entity, run_node);
	next_node = rb_next(runq->rb_leftmost);
	next_se = container_of(next_node, struct sched_entity, run_node);
	if (next_se->jiffies_vruntime < cur_se->jiffies_vruntime) {
		dequeue_se(cur_se);
		enqueue_se(cur_se);
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
