#ifndef _KTIMER_H_
#define _KTIMER_H_

#include <rbtree.h>
#include <stdint-gcc.h>

typedef void (*timer_handler)(uint32_t el);
typedef uint64_t (*csd_get_current_tick)(void);

struct clock_source_device {
	uint64_t current_tick;
	uint64_t current_time;
};

struct timer_root {
	struct rb_root root;
	struct rb_node *rb_leftmost;
};

enum timer_type {
	ONESHOT_TIMER = 0,
	SCHED_TIMER,
	REALTIME_TIMER,
};

struct timer_struct {
	uint32_t tc;
	uint32_t intvl;
	timer_handler handler;
	struct task_struct *pt;
	int type;
	struct rb_node run_node;
	void *arg;
};

void do_timer(uint32_t elapsed);
void timertree_init(void);
void insert_timer(struct timer_root *ptr, struct timer_struct *pts);
void sched_timer_handler(uint32_t elapsed);
void sched_timer_init(void);
void del_timer(struct timer_root *ptr, struct timer_struct *pts);
void update_sched_timer(void);
void create_rt_timer(struct task_struct *rt_task, uint32_t msec, void *arg);
void update_timer_tree(uint32_t elapsed);
uint32_t get_elapsedtime(void);

#endif
