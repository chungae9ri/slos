// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

#include <runq.h>
#include <percpu.h>

extern void remove_from_wq(struct task_struct *p);

/* from lwn.net/Articles/184495 */
static void enqueue_se(struct sched_entity *se)
{
	struct cfs_rq *this_runq = NULL;
	struct rb_node **link, *parent = NULL;
	uint64_t value = se->jiffies_vruntime;
	int leftmost = 1;

#if _ENABLE_SMP_
	this_runq = __get_cpu_var(runq);
#else
	this_runq = runq;
#endif
	link = &this_runq->root.rb_node;

	/* Go to the bottom of the tree */
	while (*link) {
		parent = *link;
		struct sched_entity *entry = rb_entry(parent, struct sched_entity, run_node);

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
		this_runq->rb_leftmost = &se->run_node;

	/* Put the new node there */
	rb_link_node(&se->run_node, parent, link);
	rb_insert_color(&se->run_node, &this_runq->root);
}

void init_rq(void)
{
	struct task_struct *this_idle_task = NULL;
	struct cfs_rq *this_runq = NULL;
#if _ENABLE_SMP_
	this_idle_task = __get_cpu_var(idle_task);
	__get_cpu_var(runq) = (struct cfs_rq *)kmalloc(sizeof(struct cfs_rq));
	this_runq = __get_cpu_var(runq);
#else
	this_idle_task = idle_task;
	runq = this_runq = (struct cfs_rq *)kmalloc(sizeof(struct cfs_rq));
#endif
	this_runq->root = RB_ROOT;
	this_runq->priority_sum = 0;
	this_runq->rb_leftmost = &this_idle_task->se.run_node;
	this_runq->cfs_task_num = 0;
	this_runq->curr = this_runq->next = this_runq->last = NULL;
	this_runq->root.rb_node = NULL;

	rb_init_node(&this_idle_task->se.run_node);
	enqueue_se_to_runq(&this_idle_task->se);
	this_runq->cfs_task_num++;
}

void dequeue_se(struct sched_entity *se)
{
	struct cfs_rq *this_runq = NULL;
#if _ENABLE_SMP_
	this_runq = __get_cpu_var(runq);
#else
	this_runq = runq;
#endif
	if (this_runq->rb_leftmost == (struct rb_node *)&se->run_node) {
		struct rb_node *next_node;

		next_node = rb_next(&se->run_node);
		this_runq->rb_leftmost = next_node;
	}

	rb_erase(&se->run_node, &this_runq->root);
}

void update_vruntime_runq(struct sched_entity *se)
{
	struct cfs_rq *this_runq = NULL;
	struct sched_entity *cur_se, *se_leftmost;
	struct rb_node *cur_rb_node;
#if _ENABLE_SMP_
	this_runq = __get_cpu_var(runq);
#else
	this_runq = runq;
#endif
	/* in very first time, the leftmost should be null */
	if (!this_runq->rb_leftmost)
		return;

	cur_rb_node = this_runq->rb_leftmost;
	se_leftmost = container_of(cur_rb_node, struct sched_entity, run_node);
	cur_se = se_leftmost;

	se->jiffies_consumed = (se_leftmost->jiffies_vruntime) >> (se->pri_div_shift);
	se->jiffies_vruntime = se->jiffies_consumed * se->priority;

	while (cur_se) {
		cur_se->jiffies_consumed =
		    (se_leftmost->jiffies_vruntime) >> (cur_se->pri_div_shift);
		cur_se->jiffies_vruntime = cur_se->jiffies_consumed * cur_se->priority;

		cur_rb_node = rb_next(&cur_se->run_node);
		if (cur_rb_node)
			cur_se = container_of(cur_rb_node, struct sched_entity, run_node);
		else
			cur_se = NULL;
	}
}

void enqueue_se_to_runq(struct sched_entity *se)
{
	/*struct sched_entity *se_leftmost;*/
	struct task_struct *tp = NULL;
	struct cfs_rq *this_runq = NULL;
	uint32_t *this_rqlock = NULL;
#if _ENABLE_SMP_
	this_runq = __get_cpu_var(runq);
	this_rqlock = (uint32_t *)__get_cpu_var_addr(rqlock);
#else
	this_runq = runq;
	this_rqlock = &rqlock;
#endif
	tp = container_of(se, struct task_struct, se);
	this_runq->priority_sum += se->priority;
	remove_from_wq(tp);
	tp->state = TASK_RUNNING;
	spin_lock_acquire(this_rqlock);
	update_vruntime_runq(se);
	enqueue_se(se);
	spin_lock_release(this_rqlock);
}

void update_se(uint32_t elapsed)
{
	struct sched_entity *next_se = NULL, *cur_se = NULL;
	struct rb_node *next_node = NULL;
	struct cfs_rq *this_runq = NULL;

#if _ENABLE_SMP_
	this_runq = __get_cpu_var(runq);
#else
	this_runq = runq;
#endif

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
	cur_se = container_of(this_runq->rb_leftmost, struct sched_entity, run_node);
	next_node = rb_next(this_runq->rb_leftmost);
	next_se = container_of(next_node, struct sched_entity, run_node);
	if (next_se->jiffies_vruntime < cur_se->jiffies_vruntime) {
		dequeue_se(cur_se);
		enqueue_se(cur_se);
	}
#endif
}
