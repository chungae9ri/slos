/*
  kernel/core/runq.c process management
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
#include <runq.h>

volatile uint32_t rqlock = 0;
struct cfs_rq *runq = NULL;

extern struct task_struct *idle_task;
extern void remove_from_wq(struct task_struct *p);

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

