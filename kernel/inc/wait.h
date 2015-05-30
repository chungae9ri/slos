#include <stdint-gcc.h>
#include <task.h>

struct wait_queue_t {
	int magic;
	volatile uint32_t queuelock;
	struct list_head task_list;
	int count;
};

void init_waitq(struct wait_queue_t *wq);
void add_to_waitq(struct wait_queue_t *wq, struct task_struct *tp);
void remove_from_waitq(struct wait_queue_t *wq, struct task_struct *tp);
