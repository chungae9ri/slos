#include <stdint-gcc.h>
#include <task.h>

struct wait_queue {
	int magic;
	volatile uint32_t queuelock;
	struct list_head task_list;
	int count;
};

void init_wq(void);
void add_to_wq(struct task_struct *tp);
void remove_from_wq(struct task_struct *tp);
