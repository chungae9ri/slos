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

#define XPAR_PS7_SCUTIMER_0_BASEADDR 0xF8F00600
#define PRIV_TMR_LD		     (XPAR_PS7_SCUTIMER_0_BASEADDR + 0x0000)
#define PRIV_TMR_CNTR		     (XPAR_PS7_SCUTIMER_0_BASEADDR + 0x0004)
#define PRIV_TMR_CTRL		     (XPAR_PS7_SCUTIMER_0_BASEADDR + 0x0008)
#define PRIV_TMR_INTSTAT	     (XPAR_PS7_SCUTIMER_0_BASEADDR + 0x000C)
#define PRIV_TMR_EN_MASK	     0x00000001
#define PRIV_TMR_AUTO_RE_MASK	     0x00000002
#define PRIV_TMR_IRQ_EN_MASK	     0x00000004
#define PRIV_TMR_PRESCL_MASK	     0x0000FF00
#define XPS_SCU_TMR_INT_ID	     29U /* SCU Private Timer interrupt */
#define PRIV_TMR_INT_VEC	     XPS_SCU_TMR_INT_ID

#define XPAR_XTTCPS_0_BASEADDR 0xF8001000U
#define SPI_TTC0_0_CLK_CTRL    (XPAR_XTTCPS_0_BASEADDR + 0x0000)
#define SPI_TTC0_1_CLK_CTRL    (XPAR_XTTCPS_0_BASEADDR + 0x0004)
#define SPI_TTC0_2_CLK_CTRL    (XPAR_XTTCPS_0_BASEADDR + 0x0008)

#define SPI_TTC0_0_CNT_CTRL (XPAR_XTTCPS_0_BASEADDR + 0x000C)
#define SPI_TTC0_1_CNT_CTRL (XPAR_XTTCPS_0_BASEADDR + 0x0010)
#define SPI_TTC0_2_CNT_CTRL (XPAR_XTTCPS_0_BASEADDR + 0x0014)

#define XPAR_CPU_CORTEXA9_0_CPU_CLK_FREQ_HZ 666666687

/**
 * @brief Nonblocking msec delay
 *
 * This suspend current task for input msec and yield cpu to next task
 *
 * @param [in] msecs msec duration
 */
void mdelay(uint32_t msecs);

/**
 * @brief Nonblocking usec delay
 *
 * This suspend current task for input usec and yield cpu to next task
 *
 * @param [in] usecs usec duration
 */
void udelay(uint32_t usecs);

/**
 * @brief Get the ticks per sec
 *
 * @return uint32_t Tick counter per sec
 */
uint32_t get_ticks_per_sec(void);

/**
 * @brief Set the ticks per sec
 *
 * @param [in] tps Ticks per sec value
 */
void set_ticks_per_sec(uint32_t tps);

/**
 * @brief Get the clock device freq
 *
 * @return uint32_t Clock freq
 */
uint32_t get_timer_freq(void);

/**
 * @brief Timer ISR
 *
 * @param [in] arg Argument
 * @return int32_t 0 for success
 */
int32_t timer_irq(void *arg);

/**
 * @brief Get current clock device tick count
 *
 * @return uint32_t Current tick count
 */
uint32_t timer_get_phy_tick_cnt(void);

/**
 * @brief Initialize timer clock device
 *
 */
void init_timer(void);

/**
 * @brief Enable timer clock device
 *
 */
void timer_enable(void);

/**
 * @brief Enable secondary timer clock device
 *
 */
void timer_enable_secondary(void);
#endif

/**
 * @}
 * @}
 * @}
 *
 */
