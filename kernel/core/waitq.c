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
 * @brief task waitq management functions, initialization, enqueue, dequeue
 *
 */

#include <task.h>
#include <waitq.h>
#include <defs.h>
#include <runq.h>
#include <percpu.h>
#include <ops.h>

void init_wq(void)
{
	struct wait_queue *this_wq;

#if _ENABLE_SMP_
	__get_cpu_var(wq) = kmalloc(sizeof(struct wait_queue));
	this_wq = __get_cpu_var(wq);
#else
	this_wq = wq = kmalloc(sizeof(struct wait_queue));
#endif
	this_wq->wqlock = 0;
	this_wq->magic = 1;
	this_wq->task_list.next = NULL;
	this_wq->task_list.prev = NULL;
	this_wq->count = 0;
}

void wake_all_wq(void)
{
	struct list_head *cur, *next;
	struct wait_queue *this_wq;

#if _ENABLE_SMP_
	this_wq = __get_cpu_var(wq);
#else
	this_wq = wq;
#endif
	if (!this_wq) {
		return;
	}

	cur = this_wq->task_list.next;
	while (cur) {
		next = cur->next;
		cur->next = NULL;
		cur->prev = NULL;
		cur = next;
	}
	this_wq->count = 0;
}

void destroy_wq(void)
{
	struct wait_queue *this_wq;

#if _ENABLE_SMP_
	this_wq = __get_cpu_var(wq);
#else
	this_wq = wq;
#endif
	if (!this_wq) {
		return;
	}

	wake_all_wq();
	this_wq->magic = 0;
}

void add_to_wq(struct task_struct *p)
{
	struct list_head *temp_p;
	struct wait_queue *this_wq;

#if _ENABLE_SMP_
	this_wq = __get_cpu_var(wq);
#else
	this_wq = wq;
#endif

	if (!this_wq) {
		return;
	}

	temp_p = this_wq->task_list.next;

	spin_lock_acquire(&this_wq->wqlock);
	/* insert p into the first place */
	this_wq->task_list.next = &p->waitlist;
	p->waitlist.prev = &this_wq->task_list;
	p->waitlist.next = temp_p;
	this_wq->count++;
	spin_lock_release(&this_wq->wqlock);
}

void remove_from_wq(struct task_struct *p)
{
	struct list_head *cur_p;
	struct task_struct *pt;
	struct wait_queue *this_wq = NULL;

#if _ENABLE_SMP_
	this_wq = __get_cpu_var(wq);
#else
	this_wq = wq;
#endif

	if (!this_wq) {
		return;
	}

	spin_lock_acquire(&this_wq->wqlock);
	cur_p = this_wq->task_list.next;
	while (cur_p) {
		pt = container_of(cur_p, struct task_struct, waitlist);
		if (pt == p) {
			pt->waitlist.prev->next = pt->waitlist.next;
			if (pt->waitlist.next != NULL) {
				pt->waitlist.next->prev = pt->waitlist.prev;
			}
			pt->waitlist.prev = NULL;
			pt->waitlist.next = NULL;
			this_wq->count--;
			break;
		}

		cur_p = cur_p->next;
	}
	spin_lock_release(&this_wq->wqlock);
}

int wait_queue_block(unsigned long timeout)
{
	return 0;
}

int wait_queue_wake_one(bool resched)
{
	return 0;
}

int wait_queue_wake_all(bool resched)
{
	return 0;
}

void dequeue_se_to_wq(struct sched_entity *se)
{
	uint32_t *this_rqlock = NULL;
	struct task_struct *tp = NULL;
	struct cfs_rq *this_runq = NULL;

#if _ENABLE_SMP_
	this_runq = __get_cpu_var(runq);
	this_rqlock = (uint32_t *)__get_cpu_var_addr(rqlock);
#else
	this_runq = runq;
	this_rqlock = &rqlock;
#endif
	if (!this_runq) {
		return;
	}

	tp = container_of(se, struct task_struct, se);
	this_runq->priority_sum -= se->priority;
	add_to_wq(tp);
	tp->state = TASK_WAITING;

	spin_lock_acquire(this_rqlock);
	update_vruntime_runq(se);
	dequeue_se(se);
	spin_lock_release(this_rqlock);
}

void dequeue_se_to_exit(struct sched_entity *se)
{
	struct task_struct *tp;
	struct cfs_rq *this_runq;
	uint32_t *this_rqlock = NULL;

#if _ENABLE_SMP_
	this_runq = __get_cpu_var(runq);
	this_rqlock = (uint32_t *)__get_cpu_var_addr(rqlock);
#else
	this_runq = runq;
	this_rqlock = &rqlock;
#endif
	if (!this_runq) {
		return;
	}

	tp = container_of(se, struct task_struct, se);
	if (tp->state == TASK_RUNNING || tp->state == TASK_STOP_RUNNING) {
		this_runq->priority_sum -= se->priority;
		spin_lock_acquire(this_rqlock);
		update_vruntime_runq(se);
		dequeue_se(se);
		spin_lock_release(this_rqlock);
		tp->state = TASK_STOP;
	} else if (tp->state == TASK_WAITING) {
		spin_lock_acquire(this_rqlock);
		remove_from_wq(tp);
		spin_lock_release(this_rqlock);
		tp->state = TASK_STOP;
	}
}

/**
 * @}
 * @}
 * @}
 *
 */
