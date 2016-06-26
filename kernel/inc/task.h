#ifndef __TASK_H__
#define __TASK_H__

#include <stdbool.h>
#include <stdint-gcc.h>
#include <stddef.h>
#include <rbtree.h>
#include <mem_layout.h>

#define MAX_TASK	((SVC_STACK_BASE-SYS_STACK_BASE)/(TASK_STACK_GAP))
#define MAX_USR_TASK	5 /* max user task num is 5 */

enum {
	USR_TASK0=0,
	USR_TASK1,
	USR_TASK2,
	USR_TASK3,
	USR_TASK4
};

enum {
	TASK_RUNNING,
	TASK_WAITING,
	TASK_STOP_RUNNING,
	TASK_STOP
};

struct list_head {
	struct list_head *prev, *next;
};

typedef int (*task_entry)(void);

struct sched_entity {
	uint64_t vruntime;
	uint64_t jiffies_consumed;
	struct rb_node run_node;
	uint32_t priority;
};

struct cfs_rq {
	struct sched_entity *curr, *next, *last;
	struct rb_root root;
	struct rb_node *rb_leftmost;
	uint32_t priority_sum;
};
/* do not change the order */
struct task_context_struct {
	uint32_t pc;
	uint32_t lr;
	uint32_t sp;
	uint32_t r[13];
	uint32_t spsr;
	uint32_t tlb; /* translation base address */
};

struct task_struct {
	struct task_context_struct ct;
	/*struct task_context_struct ct;*/
	task_entry entry;
	char name[32];
	struct sched_entity se;
	struct list_head task;
	struct list_head waitlist;
	uint32_t state;
};

void init_cpuidle_task(void);
void init_task(void);
void init_jiffies(void);
void init_task(void);
void create_all_task(void);
void forkyi(struct task_struct *pbt, struct task_struct *pt);
struct task_struct *do_forkyi(char *name, task_entry fn, int idx, unsigned int *ppd);
void switch_context(struct task_struct *prev, struct task_struct *next);
void schedule(void *arg);
void dequeue_se_to_exit(struct cfs_rq *rq, struct sched_entity *se);
void enqueue_se_to_runq(struct cfs_rq *rq, struct sched_entity *se, bool update);
void dequeue_se_to_waitq(struct cfs_rq *rq, struct sched_entity *se, bool update);
void drop_usrtask();
void init_idletask(unsigned int *ppd);
void init_shell(unsigned int *ppd);
void update_se();
void set_priority(struct task_struct *pt, uint32_t pri);
void put_to_sleep(char *dur, int idx);

void func1(void);
void func2(void);
static inline struct task_struct *to_task_from_listhead(struct list_head *t)
{       
	/*return container_of(t, struct task_struct, task);*/
	return ((struct task_struct *)((unsigned int)t-offsetof(struct task_struct, task)));
}

static inline struct task_struct *to_task_from_se(struct sched_entity *s)
{       
	/*return container_of(t, struct task_struct, task);*/
	return ((struct task_struct *)((unsigned int)s-offsetof(struct task_struct, se)));
}
#endif
