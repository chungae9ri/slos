/* SPDX-License-Identifier: MIT OR Apache-2.0
 *
 * Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>
 */

#ifndef OPS_H
#define OPS_H

#include <stdint.h>

#include <sched.h>

/**
 * @brief Reset handler for secondary CPU
 */

void secondary_reset(void);
/**
 * @brief Read SCR register value
 *
 * @return uint32_t SCR register value
 */
uint32_t read_scr(void);

/**
 * @brief Flush entire data cache
 */
void flush_ent_dcache(void);

/**
 * @brief Get current processor ID
 *
 * @return uint32_t Current processor ID
 */
uint32_t smp_processor_id(void);

/**
 * @brief enable interrupt
 */
void enable_interrupt(void);

/**
 * @brief disable interrupt
 */
void disable_interrupt(void);

/**
 * @brief Switch task context
 *
 * @param current Current task context
 * @param next Next task context
 */
void do_switch_context(struct task_struct *current, struct task_struct *next);

/**
 * @brief Yield cpu to target task, switch context
 *
 * @param target Target task (yielded) context
 * @param current Current task context
 */
void switch_context_yieldyi(struct task_struct *target, struct task_struct *current);

/**
 * @brief Acquire spinlock
 *
 * @param pl Pointer to a flag in memory for exclusive access
 */
void spin_lock_acquire(uint32_t *pl);

/**
 * @brief Release spinlock
 *
 * @param pl Pointer to a flag in memory for exclusive access
 */
void spin_lock_release(uint32_t *pl);
#endif
