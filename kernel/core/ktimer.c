#include <ktimer.h>
#include <timer.h>
#include <task.h>
#include <stdlib.h>
#include <defs.h>
#include <smm.h>

struct clock_source_device csd;
struct timer_root *ptroot = NULL;
struct timer_struct *sched_timer = NULL;
uint64_t jiffies;
extern struct task_struct *current;
volatile uint32_t timertree_lock;
extern void spin_lock_acquire(volatile uint32_t *lock);
extern void spin_lock_release(volatile uint32_t *lock);

void timertree_init(void)
{
	ptroot = (struct timer_root *) kmalloc(sizeof(struct timer_root));
	ptroot->root = RB_ROOT;
}

void update_csd(void)
{
	csd.current_tick = timer_get_phy_tick_cnt();
	/*csd.current_time = (uint64_t)(csd.current_tick / get_ticks_per_sec());*/
}

uint32_t get_elapsedtime()
{
	uint32_t elapsed;
	uint64_t tick_prev = csd.current_tick;

	update_csd();
	return (uint32_t)(csd.current_tick - tick_prev);
}

void update_timer_tree(uint32_t elapsed)
{
	struct rb_node *pcur = NULL;
	struct timer_struct *pct = NULL;

	pcur = ptroot->rb_leftmost;
	while (pcur != NULL) {
		pct = container_of(pcur, struct timer_struct, run_node);
		if (pcur == ptroot->rb_leftmost) {
			pct->tc = pct->intvl;
		} else {
			if (pct->tc <= elapsed) {
				if (pct->type == REALTIME_TIMER) {
					pct->pt->missed_cnt++;
				}
			} else {
				pct->tc = pct->tc - elapsed;
			}
		}

		del_timer(ptroot, pct);
		insert_timer(ptroot, pct);
		pcur = rb_next(pcur);
	}
}

void do_timer(uint32_t elapsed)
{
	/*if (!current || jiffies <= 100) return;*/
	update_se(elapsed);
	schedule();
}

void update_sched_timer(void)
{
	uint32_t elapsed;
	uint64_t tick_prev = csd.current_tick;
	update_csd();
	elapsed = (uint32_t)((csd.current_tick - tick_prev) & 0xffffffff);
	sched_timer->tc -= elapsed;
	/* don't need this. fixed */
#if 0 
	/* if next sched tick is under 5msec */
	if (elapsed >= get_ticks_per_sec()/200) {
		sched_timer->tc = get_ticks_per_sec()/100; /* set to default value */
	} else {
		sched_timer->tc -= elapsed;
	}
#endif
}

void sched_timer_handler(uint32_t elapsed)
{
	jiffies++;	
	do_timer(elapsed);
}

void create_rt_timer(struct task_struct *rt_task, uint32_t msec, void *arg)
{
	struct timer_struct *rt_timer = (struct timer_struct *)kmalloc(sizeof(struct timer_struct));
	rt_timer->pt = rt_task;
	rt_timer->handler = NULL;
	rt_timer->type = REALTIME_TIMER;
	rt_timer->tc = get_ticks_per_sec() / 1000 * msec;
	rt_timer->intvl = rt_timer->tc;
	rt_timer->arg = arg;

	insert_timer(ptroot, rt_timer);
}

void create_oneshot_timer(timer_handler oneshot_timer_handler, uint32_t msec, void *arg)
{
	struct timer_struct *oneshot_timer = (struct timer_struct *)kmalloc(sizeof(struct timer_struct));
	oneshot_timer->handler = oneshot_timer_handler;
	oneshot_timer->type = ONESHOT_TIMER;
	oneshot_timer->tc = get_ticks_per_sec() / 1000 * msec;
	oneshot_timer->arg = arg;

	insert_timer(ptroot, oneshot_timer);
}

void sched_timer_init(void)
{
	sched_timer = (struct timer_struct *)kmalloc(sizeof(struct timer_struct));
	sched_timer->handler = sched_timer_handler;
	sched_timer->type = SCHED_TIMER;
	/* 10msec sched timer tick */
	sched_timer->tc = get_ticks_per_sec()/100;
	sched_timer->intvl = sched_timer->tc;

	jiffies = 0;
	insert_timer(ptroot, sched_timer);
}

/* thread safe timer tree insert */
void insert_timer(struct timer_root *ptr, struct timer_struct *pts)
{
	struct rb_node **link = &ptr->root.rb_node, *parent = NULL;
	uint64_t value = pts->tc;
	int leftmost = 1;

	spin_lock_acquire(&timertree_lock);
	/* Go to the bottom of the tree */
	while (*link) {
		parent = *link;
		struct timer_struct *entry = rb_entry(parent, struct timer_struct, run_node);
		if (entry->tc > value)
			link = &(*link)->rb_left;
		else /* if (entry->tc<= value)*/ {
			link = &(*link)->rb_right;
			leftmost = 0;
		}
	}
	/* Maintain a cache of leftmost tree entries */

	if (leftmost)
		ptr->rb_leftmost = &pts->run_node;
	/* put the new node there */
	rb_link_node(&pts->run_node, parent, link);
	rb_insert_color(&pts->run_node, &ptr->root);
	spin_lock_release(&timertree_lock);
}

void del_timer(struct timer_root *ptr, struct timer_struct *pts)
{
	if (ptr->rb_leftmost == (struct rb_node *)&pts->run_node) {
		struct rb_node *next_node;

		next_node = rb_next(&pts->run_node);
		ptr->rb_leftmost = next_node;
	}

	rb_erase(&pts->run_node, &ptr->root);
}

