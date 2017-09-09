#ifndef _KTIMER_H_
#define _KTIMER_H_

#include <rbtree.h>
#include <stdint-gcc.h>

typedef void (*timer_handler)(uint32_t el);

struct clock_source_device {
	uint32_t current_tick;
	uint32_t current_time;
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
	uint32_t type;
	struct rb_node run_node;
	uint32_t idx;
	void *arg;
};

void timertree_init(void);
void create_sched_timer(timer_handler sched_handler, uint32_t msec, uint32_t idx, void *arg);
void insert_timer(struct timer_root *ptr, struct timer_struct *pts);
void del_timer(struct timer_root *ptr, struct timer_struct *pts);
void sched_timer_handler(uint32_t elapsed);
void create_rt_timer(timer_handler rt_handler, uint32_t msec, uint32_t idx, void *arg);
void update_timer_tree(uint32_t elapsed);
uint32_t get_elapsedtime(void);

#endif
