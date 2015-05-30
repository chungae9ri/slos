#ifndef _KTIMER_H_
#define _KTIMER_H_

#include <rbtree.h>
#include <stdint-gcc.h>

typedef void (*timer_handler)(void *arg);
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
};

struct timer_struct {
	uint32_t tc;
	uint64_t tc_prev;
	timer_handler handler;
	int type;
	struct rb_node run_node;
	void *arg;
};

void do_timer(void *arg);
void timertree_init(void);
void insert_timer(struct timer_root *ptr, struct timer_struct *pts);
void sched_timer_handler(void *arg);
void sched_timer_init(void);
void del_timer(struct timer_root *ptr, struct timer_struct *pts);
void update_sched_timer(void);

#endif
