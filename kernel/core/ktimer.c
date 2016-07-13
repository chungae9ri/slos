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

void timertree_init(void)
{
	ptroot = (struct timer_root *) kmalloc(sizeof(struct timer_root));
	ptroot->root = RB_ROOT;
}

void update_csd(void)
{
	csd.current_tick = timer_get_phy_tick_cnt();
	csd.current_time = (uint64_t)(csd.current_tick / get_ticks_per_sec());
}

void update_oneshot_timer(void)
{
	struct rb_node *pcur=NULL;
	struct timer_struct *pct = NULL;

	pcur = ptroot->rb_leftmost;
	pct = container_of(pcur, struct timer_struct, run_node);
	while (pcur != NULL) {

		if (pct->type == ONESHOT_TIMER) {
			pct->tc = pct->tc - sched_timer->tc;
			if (pct->tc < sched_timer->tc) {
				del_timer(ptroot, pct);
				insert_timer(ptroot, pct);
			}
		}
		
		pcur = rb_next(pcur);
		pct = container_of(pcur, struct timer_struct, run_node);
	}
}

void do_timer(void *arg)
{
	if (!current || jiffies <= 100) return;
	update_csd();
	update_oneshot_timer();
	update_se();
	schedule(arg);
}

void update_sched_timer(void)
{
	uint32_t elapsed;
	uint64_t tick_prev = csd.current_tick;
	update_csd();
	elapsed = (uint32_t)((csd.current_tick - tick_prev) & 0xffffffff);
	sched_timer->tc -= elapsed;
	/* if next sched tick is under 5msec */
	if (elapsed >= get_ticks_per_sec()/200) {
		sched_timer->tc = get_ticks_per_sec()/100; /* set to default value */
	} else {
		sched_timer->tc -= elapsed;
	}
}

void sched_timer_handler(void *arg)
{
	jiffies++;	
	do_timer(arg);
}

void create_oneshot_timer(timer_handler oneshot_timer_handler, uint32_t msec, void *arg)
{
	struct timer_struct *oneshot_timer = (struct timer_struct *)kmalloc(sizeof(struct timer_struct));
	oneshot_timer->handler = oneshot_timer_handler;
	oneshot_timer->type = ONESHOT_TIMER;
	oneshot_timer->tc = get_ticks_per_sec()/1000 * msec;
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

	jiffies = 0;
	insert_timer(ptroot, sched_timer);
}

void insert_timer(struct timer_root *ptr, struct timer_struct *pts)
{
	struct rb_node **link = &ptr->root.rb_node, *parent = NULL;
	uint64_t value = pts->tc;
	int leftmost = 1;

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

