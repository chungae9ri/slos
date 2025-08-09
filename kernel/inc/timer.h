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
 * @brief Kernel clock device
 *
 */

#ifndef _TIMER_H_
#define _TIMER_H_

#include <stdint.h>

#include <errno.h>
#include <regops.h>
#include <device.h>

/** Export GIC device */
DEVICE_DECLARE_IDX(timer, 0);

#define PRIV_TMR_LD_OFFSET	(0x0000)
#define PRIV_TMR_CNTR_OFFSET	(0x0004)
#define PRIV_TMR_CTRL_OFFSET	(0x0008)
#define PRIV_TMR_INTSTAT_OFFSET (0x000C)
#define PRIV_TMR_EN_MASK	0x00000001
#define PRIV_TMR_AUTO_RE_MASK	0x00000002
#define PRIV_TMR_IRQ_EN_MASK	0x00000004
#define PRIV_TMR_PRESCL_MASK	0x0000FF00

/**
 * @brief Nonblocking msec delay
 *
 * This suspend current task for input msec and yield cpu to next task
 *
 * @param [in] msecs msec duration
 */
void mdelay(uint32_t msecs);

/**
 * @brief Get the ticks per sec
 *
 * @return uint32_t Tick counter per sec
 */
uint32_t get_ticks_per_sec(void);

/**
 * @brief Timer ISR
 *
 * @param [in] arg Argument
 * @return int32_t 0 for success
 */
int32_t timer_irq(void *arg);

/**
 * @brief Initialize timer clock device
 *
 * @param dev Timer device controller instance
 * @return int32_t 0 for success
 */
int32_t init_timer(struct device *dev);

/**
 * @brief Enable timer clock device
 *
 * This enables timer clock for each CPU0 and CPU1
 * Private timer is banked per CPU.
 *
 * @param dev Timer device controller instance
 * @return int32_t 0 for success, others for failure
 */
int32_t timer_enable(const struct device *dev);

/**
 * @brief Disable timer clock device
 *
 * @param dev Timer device controller instance
 * @return int32_t 0 for success, others for failure
 */
int32_t timer_disable(const struct device *dev);

/**
 * @brief Enable secondary timer clock device
 *
 * @param dev Timer device controller instance
 */
void timer_enable_secondary(const struct device *dev);

/**
 * @brief Get current clock device tick count
 *
 * @param dev Timer device controller instance
 * @return uint32_t Current tick count
 */
static inline uint32_t timer_get_phy_tick_cnt(const struct device *dev)
{
	if (dev == NULL) {
		return 0;
	}

	return (uint32_t)read32(dev->base_addr + PRIV_TMR_CNTR_OFFSET);
}
#endif

/**
 * @}
 * @}
 * @}
 *
 */
