#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint-gcc.h>
#include <mem_layout.h>
#include <task.h>
#include <stdlib.h>
#include <timer.h>
#include <debug.h>
#include <wait.h>
#include <defs.h>
#include <loader.h>
#include <ktimer.h>
#include <msgQ.h>
#include <smm.h>
#include <xlibs.h>

#define TIMER_TEST
#define WAIT_Q_TEST


#define SVCSPSR 0x13 

uint32_t task_created_num = 1; /* cpuidle task is not created by do_forkyi. it is already made from start */
struct task_struct task_arr[MAX_TASK];
extern void do_switch_context(struct task_struct *, struct task_struct *);
extern void switch_context_yield(struct task_struct *, struct task_struct *);
extern uint64_t	jiffies;
struct task_struct *current = NULL;
struct task_struct *last = NULL;
struct task_struct *first = NULL;
struct task_struct idle_task;
extern void cpuidle(void);
struct cfs_rq *runq = NULL;
uint32_t show_stat = 0;
struct wait_queue_t wq_sched;
struct task_struct *upt[MAX_USR_TASK]={NULL, };
#ifdef WAIT_Q_TEST
struct task_struct *fortest1;
#endif
extern struct timer_root *ptroot;

extern char *exec;
volatile uint32_t rqlock;
void oneshot_user_timer_handler(void *arg);
extern void spin_lock_acquire_irqsafe(volatile uint32_t *pl);
extern void spin_lock_release_irqsafe(volatile uint32_t *pl);
extern int uart_getc(int port, int wait);
extern void create_oneshot_timer(timer_handler oneshot_timer_handler, uint32_t msec, void *arg);

#ifdef TIMER_TEST
void oneshot_timer_testhandler(void *arg)
{
	/* caution : since this handler runs in interrupt context, 
	   don't use print_msg
	 */
	/*print_msg("I am called by oneshot timer test handler!! \r\n");*/
	int i;
	i++;
}
#endif

#ifdef USE_MMU
#else
void create_rt_task(char *name, task_entry handler, int dur)
#endif
{
	struct task_struct *temp;

	temp = do_forkyi(name, (task_entry)handler, -1, RT_TASK);
	create_rt_timer(temp, dur, NULL);
}

void rt_task_handler2(void *arg)
{
	int i, j = 10;
	while (1) {
		/* do some real time work here */
		for (i = 0; i < 10; i++) {
			j++;
		}
		if (show_stat) 	print_msg("I am rt worker2 \n");
		/* should yield after finish current work */
		yield();
	}
}

void rt_task_handler(void *arg)
{
	int i, j = 10;
	while (1) {
		/* do some real time work here */
		for (i = 0; i < 10; i++) {
			j++;
		}
		if (show_stat) 	print_msg("I am rt worker\n");
		/* should yield after finish current work */
		yield();
	}
}

extern void enable_interrupt();
extern void disable_interrupt();
extern void update_csd(void);

void yield()
{
	uint32_t elapsed;
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
	current = current->yield_task;
	switch_context_yield(temp, current);
}

void func1(void )
{
	while (1) {
		if (show_stat) print_msg("dummy1 running....\r\n");
		/*put_dummy1_to_sleep();*/
	}
}

void worker(void)
{
	int val;
	while (1) {
		if (show_stat) print_msg("worker running....\r\n");
#if 0
		val = getmsg(MSG_USR0, false);
		if (val != 0) {
			dequeue_se_to_waitq(runq, &upt[MSG_USR0-1]->se, true);
			create_oneshot_timer(oneshot_user_timer_handler, val, MSG_USR0-1);
			setmsg(0, MSG_USR0, false);
		}
#endif
	}
}

void init_idle_task()
{
	strcpy(idle_task.name, "idle_task");
	idle_task.entry = (task_entry)cpuidle;
}

void init_jiffies()
{
	jiffies = 0;
}

void init_task()
{
	int i;
	strcpy(task_arr[0].name,"idle task");

	for (i=1 ; i<MAX_TASK ; i++) {
		sprintf(task_arr[i].name,"task:%d",i);
		if(i==1) task_arr[i].entry = (task_entry)func1;
		else if (i==2) task_arr[i].entry = (task_entry)worker;
	}

	create_all_task();
}

void drop_usrtask()
{
	int i;
	for (i=0 ; i<MAX_USR_TASK ; i++) {
		if ( upt[i] != NULL && upt[i]->state == TASK_STOP_RUNNING) {
			dequeue_se_to_exit(runq, &(upt[i]->se));
			task_created_num--;
			upt[i]->task.prev->next = upt[i]->task.next;
			upt[i]->task.next->prev = upt[i]->task.prev;
			if (upt[i] == last) last = container_of(upt[i]->task.prev, struct task_struct, task);
			upt[i] = NULL;
			print_msg("\r\nfree user app");
		}
	}
}

/* from lwn.net/Articles/184495 */
void enqueue_se(struct cfs_rq *rq, struct sched_entity *se)
{
	struct rb_node **link = &rq->root.rb_node, *parent=NULL;
	uint64_t value = se->vruntime;
	int leftmost = 1;

	/* Go to the bottom of the tree */
	while (*link) {
		parent = *link;
		struct sched_entity *entry= rb_entry(parent, struct sched_entity, run_node);

		if (entry->vruntime > value)
			link = &(*link)->rb_left;
		else if (entry->vruntime < value) {
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
	se->ticks_consumed = se_leftmost->vruntime * rq->priority_sum / se->priority;
	se->vruntime = se->ticks_consumed * se->priority / rq->priority_sum;

	while (cur_se) {
		cur_se->ticks_consumed = se_leftmost->vruntime * rq->priority_sum / cur_se->priority;
		cur_se->vruntime = cur_se->ticks_consumed * cur_se->priority / rq->priority_sum;
		cur_rb_node = rb_next(&cur_se->run_node);
		if (cur_rb_node)
			cur_se = container_of(cur_rb_node, struct sched_entity, run_node);
		else cur_se = NULL;
	}
}

void enqueue_se_to_runq(struct cfs_rq *rq, struct sched_entity *se, bool update)
{
	struct sched_entity *se_leftmost;
	struct task_struct *tp;

	if (update) {
		tp = container_of(se, struct task_struct, se);
		rq->priority_sum += se->priority;
		remove_from_waitq(&wq_sched, tp);
		tp->state = TASK_RUNNING;
		spin_lock_acquire_irqsafe(&rqlock);
		update_vruntime_runq(rq, se);
		spin_lock_release_irqsafe(&rqlock);

	}

	spin_lock_acquire_irqsafe(&rqlock);
	enqueue_se(rq, se);
	spin_lock_release_irqsafe(&rqlock);
}

void dequeue_se_to_exit(struct cfs_rq *rq, struct sched_entity *se)
{
	struct task_struct *tp;
	tp = container_of(se, struct task_struct, se);
	if (tp->state == TASK_RUNNING || tp->state == TASK_STOP_RUNNING){
		rq->priority_sum -= se->priority;
		spin_lock_acquire_irqsafe(&rqlock);
		update_vruntime_runq(rq, se);
		dequeue_se(rq, se);
		spin_lock_release_irqsafe(&rqlock);
		tp->state = TASK_STOP;
	} else if (tp->state == TASK_WAITING) {
		spin_lock_acquire_irqsafe(&rqlock);
		remove_from_waitq(&wq_sched, tp);
		spin_lock_release_irqsafe(&rqlock);
		tp->state = TASK_STOP;
	}
}

void dequeue_se_to_waitq(struct cfs_rq *rq, struct sched_entity *se, bool update)
{
	struct task_struct *tp;

	if (update) {
		tp = container_of(se, struct task_struct, se);
		rq->priority_sum -= se->priority;
		add_to_waitq(&wq_sched, tp);
		tp->state = TASK_WAITING;

		spin_lock_acquire_irqsafe(&rqlock);
		update_vruntime_runq(rq, se);
		spin_lock_release_irqsafe(&rqlock);
	}

	spin_lock_acquire_irqsafe(&rqlock);
	dequeue_se(rq, se);
	spin_lock_release_irqsafe(&rqlock);
}

void init_cfs_rq(void)
{
	runq = (struct cfs_rq *)kmalloc(sizeof(struct cfs_rq));
	runq->root = RB_ROOT;
	runq->priority_sum = 0;
}

void set_priority(struct task_struct *pt, uint32_t pri)
{
	pt->se.priority = pri;
}

void print_task_stat(void)
{
	struct task_struct *next;
	struct list_head *next_lh;
	int i, idx=0, num=0;
	char buff[128];

	for (i=0 ; i<128 ; i++) buff[i] = 0;
	if (current->type == CFS_TASK) {
		num = sprintf(buff,"\n####task:%s\n",current->name);
		idx += num;
		num = sprintf(&buff[idx], "vruntime:%lu\n",current->se.vruntime);
		idx += num;
		num = sprintf(&buff[idx], "ticks_consumed:%lu\n",current->se.ticks_consumed);
	} else if (current->type == RT_TASK) {
		num = sprintf(buff,"\n####task:%s\n",current->name);
		idx += num;
		num = sprintf(&buff[idx], "vruntime:%lu\n",current->se.vruntime);
		idx += num;
		num = sprintf(&buff[idx], "missed deadline cnt : %u\n", current->missed_cnt);
	}
		print_msg(buff);

	next = current;
	for (;;)  {
		next_lh = next->task.next;
		next = (struct task_struct *)to_task_from_listhead(next_lh);
		if (next == current || !next)
			break;

		num=0; 
		idx=0;
		for (i=0 ; i<128 ; i++) buff[i] = 0;

		if (next->type == CFS_TASK) {
			num = sprintf(&buff[idx],"task:%s\n",next->name);
			idx += num;
			num = sprintf(&buff[idx], "vruntime:%lu\n",next->se.vruntime);
			idx += num;
			num = sprintf(&buff[idx], "ticks_consumed:%lu\n",next->se.ticks_consumed);
		} else if (next->type == RT_TASK) {
			num = sprintf(buff,"task:%s\n",next->name);
			/*idx += num;*/
			/*num = sprintf(&buff[idx], "ticks_consumed:%lu\n",next->se.ticks_consumed);*/
			idx += num;
			num = sprintf(&buff[idx], "missed deadline cnt : %u\n", next->missed_cnt);
		}
		print_msg(buff);
	}
}

void shell(void)
{
	int byte;
	int i = 0;

	show_stat = 1;

	create_rt_task("real1", (task_entry)rt_task_handler, 15);
	create_rt_task("real2", (task_entry)rt_task_handler2, 17);
	while (1) {
#if 0
		if (i == 1000) {
			/*print_task_stat();*/
			i = 0;
		}
		i++;
		print_msg("I am shell\n");
#else
		byte=uart_getc(0,1);
		print_msg("I am shell\n");
		switch (byte) {
			case 'D':
			case 'd':
				show_stat = 0;
				break;
			case 'H':
			case 'h':
				print_msg("\r\nOrder sir... T:show task stat, W:show whoami, D:disable show whoami\r\n");
#ifdef TIMER_TEST
				print_msg("\r\nI:set oneshot timer in 5sec ...\r\n");
#endif
#ifdef WAIT_Q_TEST
				print_msg("z:dequeue dummy1 task, x:enqueue dummy1 task\r\n");
#endif
				break;
			case 'j':
			case 'J':
				/*create_oneshot_timer(oneshot_timer_testhandler, 5000, NULL);*/
				/*disable_interrupt();*/
				create_rt_task("real2", (task_entry)rt_task_handler2, 17);
				/*enable_interrupt();*/
				break;

			case 'i':
			case 'I':
				/*create_oneshot_timer(oneshot_timer_testhandler, 5000, NULL);*/
				/*disable_interrupt();*/
				create_rt_task("real1", (task_entry)rt_task_handler, 15);
				/*enable_interrupt();*/
				break;

			case 'T':
			case 't':
				print_task_stat();
				break;
			case 'W':
			case 'w':
				show_stat = 1;
				break;

			case 'l':
			case 'L':
				/*load_ramdisk();*/
				break;

#ifdef WAIT_Q_TEST
			case 'z':
				dequeue_se_to_waitq(runq, &fortest1->se, true);
				break;
			case 'x':
				enqueue_se_to_runq(runq, &fortest1->se, true);
				break;
#endif
			default:
				break;
		}
		/* sleep for sometime to avoid prefetch abort */
		/*mdelay(1);*/
#endif
	}
}

#ifdef USE_MMU
void init_shell(unsigned int *ppd)
#else
void init_shell()
#endif
{
	struct task_struct *temp;

#ifdef USE_MMU
	temp = do_forkyi("shell", (task_entry)shell, -1, ppd);
#else
	temp = do_forkyi("shell", (task_entry)shell, -1, CFS_TASK);
#endif
	set_priority(temp, 2);
	rb_init_node(&temp->se.run_node);
	enqueue_se_to_runq(runq, &temp->se, true);
}

void init_rq(struct task_struct *pt)
{
	init_cfs_rq();
	rb_init_node(&pt->se.run_node);
	enqueue_se_to_runq(runq, &pt->se, true);
}

#ifdef USE_MMU
void init_idletask(unsigned int *ppd)
#else
void init_idletask()
#endif
{
	struct task_struct *pt = (struct task_struct *)kmalloc(sizeof(struct task_struct));
	struct task_struct *temp;
	strcpy(pt->name,"idle task");
	pt->task.next = NULL;
	pt->task.prev = NULL;
	pt->yield_task = NULL;
	pt->se.vruntime = 0;
	pt->se.ticks_consumed = 0;
	pt->type = CFS_TASK;
	pt->missed_cnt = 0;
	set_priority(pt, 16);
	current = first = last = pt;

	init_rq(pt);
	init_waitq(&wq_sched);

/* create some dummy tasks just for test purpose */
#ifdef USE_MMU
	temp = do_forkyi("dummy1", (task_entry)func1, -1, ppd);
#else
	temp = do_forkyi("dummy1", (task_entry)func1, -1, CFS_TASK);
#endif
#ifdef WAIT_Q_TEST
	fortest1 = temp;
#endif
	set_priority(temp, 8);
	rb_init_node(&temp->se.run_node);
	enqueue_se_to_runq(runq, &temp->se, true);
#ifdef USE_MMU
	temp = do_forkyi("worker", (task_entry)worker, -1, ppd); 
#else
	temp = do_forkyi("worker", (task_entry)worker, -1, CFS_TASK); 
#endif
	set_priority(temp, 2);
	rb_init_node(&temp->se.run_node);
	enqueue_se_to_runq(runq, &temp->se, true);
}

void create_all_task()
{
	int i;
	for (i=1 ; i<MAX_TASK ; i++) {
		forkyi(&task_arr[0], &task_arr[i]);
	}
	current = &task_arr[0]; /* current is idle task*/
}

void forkyi(struct task_struct *pbt, struct task_struct *pt)
{
	uint32_t idx;
	idx = (pt-pbt); /* automatically divided by sizeof(struct task_struct) */
	pt->ct.sp = (uint32_t)(SVC_STACK_BASE + TASK_STACK_GAP * idx);
	pt->ct.lr = (uint32_t)pt->entry;
	pt->ct.pc = (uint32_t)pt->entry;
	pt->ct.spsr = SVCSPSR;
}

#ifdef USE_MMU
struct task_struct *do_forkyi(char *name, task_entry fn, int idx, unsigned int *ppd)
#else
struct task_struct *do_forkyi(char *name, task_entry fn, int idx, TASKTYPE type)
#endif
{
	struct task_struct *pt;
	if (task_created_num == MAX_TASK) return NULL;

	/* idx >=0 for user task */
	if (idx >= 0) {
		if(upt[idx] == NULL) {
			upt[idx] = (struct task_struct *)kmalloc(sizeof(struct task_struct));
		}
		pt = upt[idx];
	} else pt = (struct task_struct *)kmalloc(sizeof(struct task_struct));

	strcpy(pt->name,name);
	pt->entry = fn;
	pt->type = type;
	pt->missed_cnt = 0;
	pt->se.vruntime = 0;
	pt->se.ticks_consumed = 0;
	pt->yield_task = NULL;
	pt->ct.sp = (uint32_t)(SVC_STACK_BASE - TASK_STACK_GAP * ++task_created_num);
	pt->ct.lr = (uint32_t)pt->entry;
	pt->ct.pc = (uint32_t)pt->entry;
	pt->ct.spsr = SVCSPSR;
#ifdef USE_MMU
	pt->ct.ttb = (unsigned long)ppd;
#endif
	/* get the last task from task list and add this task to the end of the task list*/
	last->task.next = &(pt->task);
	pt->task.prev = &(last->task);
	pt->task.next = &(first->task);
	first->task.prev = &(pt->task);
	last = pt;

	return pt;
}

void schedule()
{
	int r0 = 0;
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
	/* flush TLB */
	/*asm ("mcr p15, 0, %0, c8, c7, 0" : : "r" (r0) :);*/
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
	current->se.vruntime = (current->se.ticks_consumed) * (current->se.priority)/runq->priority_sum;

	if (&current->se.run_node == runq->rb_leftmost) {
		next_node = rb_next(&current->se.run_node);
		/* if there is only one task in runq, next_node should be NULL */
		if (!next_node) return;
	} else {
		next_node = runq->rb_leftmost;
	}
	se = rb_entry(next_node, struct sched_entity, run_node);
	if (se->vruntime < current->se.vruntime) {
		dequeue_se(runq, &current->se);
		enqueue_se(runq, &current->se);
	}
#else
	cur_se = container_of(runq->rb_leftmost, struct sched_entity, run_node);
	next_node = rb_next(runq->rb_leftmost);
	next_se = container_of(next_node, struct sched_entity, run_node);
	if (next_se->vruntime < cur_se->vruntime) {
		dequeue_se(runq, cur_se);
		enqueue_se(runq, cur_se);
	}
#endif
}

void switch_context(struct task_struct *prev, struct task_struct *next)
{
	do_switch_context(prev, next);
}

void oneshot_user_timer_handler(void *arg)
{
	int idx;
	struct rb_node *pcur = NULL;
	struct timer_struct *pct = NULL;

	print_msg("I am called by oneshot timer user handler!! \r\n");
	pcur = ptroot->rb_leftmost;
	pct = container_of(pcur, struct timer_struct, run_node);
	if (pcur != NULL) {
		idx = (int)(pct->arg);
		/*upt[idx]->state=TASK_RUNNING;*/
		enqueue_se_to_runq(runq, &upt[idx]->se, true);
	}
}

/* 2 important things are considered.
   1. current task can't put itself to sleep. The rbtree of se(sched entity) is broken.
      You need to implement yield(). fix me
   2. You can't use spin lock in exception context(interrupt, syscall...etc) which disable interrupt.
      This result in program stucking with spinlock forever.
 */
void put_to_sleep(char *dur, int idx)
{
	int sleepdur = (*dur)*100;

	if (upt[idx]->state == TASK_RUNNING) {
		/*dequeue_se_to_waitq(runq, &upt[idx]->se, true);*/
		/*create_oneshot_timer(oneshot_user_timer_handler, (*dur)*100, idx);*/
#if 0
		if (getmsg(MSG_USR0, false) == 0) setmsg(sleepdur, MSG_USR0, false);
#endif
	}
}
