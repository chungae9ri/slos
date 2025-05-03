/* SPDX-License-Identifier: MIT OR Apache-2.0
 *
 * Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>
 */

/**
 * @addtogroup kernel
 * @{
 * @addtogroup kernel_core Core
 * @{
 * @addtogroup kernel_core_proc Process management
 * @{
 *
 * @brief Kernel scheduler modlue
 *
 */

#ifndef _SCHED_H_
#define _SCHED_H_

#include <stdint.h>

#include <mem_layout.h>
#include <defs.h>
#include <rbtree.h>
#include <mm.h>

/** Task state */
enum task_state {
	TASK_RUNNING,
	TASK_WAITING,
	TASK_STOP_RUNNING,
	TASK_STOP
};

/** list_head structure */
struct list_head {
	struct list_head *prev, *next;
};

/** task entry type */
typedef uint32_t (*task_entry)(void);

/** sched entity structure */
struct sched_entity {
	uint32_t jiffies_vruntime; /**< Virtual runtime in jiffies */
	uint64_t ticks_consumed;   /**< Ticks consumed */
	uint32_t jiffies_consumed; /**< Jiffies consumed */
	struct rb_node run_node;   /**< Run node */
	uint32_t priority;	   /**< Priority of the task */
	uint32_t pri_div_shift;	   /**< Priority divider shift */
};

/** Task context structure - do not change the order */
struct task_context_struct {
	/* TODO: task ctx has r0-r11, r12 isn't included */
	uint32_t r[12];	   /**< General purpose register storage */
	uint32_t sp;	   /**< Stack pointer storage */
	uint32_t lr;	   /**< LR register storage */
	uint32_t pc;	   /**< PC storage */
	uint32_t reserved; /**< needed to align 8bytes for s0-31 registers */
	uint32_t s[32];	   /**< 32 single-precision registers (s0-s31) */
	uint32_t fpscr;	   /**< Floating-Point Status and Control Register */
	uint32_t fpexc;	   /**< Floating-Point Exception Register */
} __attribute__((aligned(8)));

/** Task type */
typedef enum {
	CFS_TASK = 0,
	RT_TASK,
	ONESHOT_TASK,
} TASKTYPE;

/** Task structure */
struct task_struct {
	struct task_context_struct ct;	/**< Task context */
	task_entry entry;		/**< Task entry */
	void *arg;			/**< Task argument */
	char name[32];			/**< Task name */
	uint32_t pid;			/**< Task pid */
	struct sched_entity se;		/**< Task sched entity */
	struct list_head task;		/**< List head in the runq */
	struct task_struct *yield_task; /**< Task preempted by this task */
	struct list_head waitlist;	/**< List head in waitq */
	TASKTYPE type;			/**< Task type */
	uint32_t missed_cnt;		/**< RT task deadline missed count */
	enum task_state state;		/**< Task state */
	uint32_t timeinterval;		/**< Realtime task time interval */
	struct mm_struct mm;		/**< Memory management for this task */
	uint32_t done;			/**<  */
	uint32_t preempted;		/**< */
};

/**
 * @brief Calculate task start address from listhead
 *
 * @param [in] t List head
 * @return struct task_struct* Pointer to task struct
 */
static inline struct task_struct *to_task_from_listhead(struct list_head *t)
{
	return ((struct task_struct *)((uint32_t)t - offsetof(struct task_struct, task)));
}

/**
 * @brief Calculate task start address from sched entity
 *
 * @param [in] s Sched entity
 * @return struct task_struct* Pointer to task struct
 */
static inline struct task_struct *to_task_from_se(struct sched_entity *s)
{
	return ((struct task_struct *)((uint32_t)s - offsetof(struct task_struct, se)));
}

/**
 * @brief Initialize CFS scheduler
 *
 */
void init_cfs_scheduler(void);

/**
 * @brief Fork new task
 *
 * @param [in] name Task name
 * @param [in] fn Task process
 * @param [in] type Task type
 * @param [in] pri Task priority
 * @return struct task_struct* Pointer to task
 */
struct task_struct *forkyi(char *name, task_entry fn, TASKTYPE type, uint32_t pri);

/**
 * @brief Switch task context
 *
 * @param [in] prev Previous task to be switched
 * @param [in] next Next task to switch to
 */
void switch_context(struct task_struct *prev, struct task_struct *next);

/**
 * @brief Schedule next task
 *
 */
void schedule(void);

/**
 * @brief Yield CPU from current task to next task
 *
 */
void yieldyi(void);

/**
 * @brief Update current task's sched entity
 *
 * @param [in] elapsed Elapsed time
 */
void update_current(uint32_t elapsed);

/**
 * @brief Print task status
 *
 * @param [in] arg Argument
 * @return int32_t 0 for success
 */
int32_t print_task_stat(void *arg);
#endif

/**
 * @}
 * @}
 * @}
 *
 */
