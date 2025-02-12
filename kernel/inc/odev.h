/* SPDX-License-Identifier: MIT OR Apache-2.0
 *
 * Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>
 */

/**
 * @addtogroup kernel
 * @{
 * @addtogroup kernel_drivers Drivers
 * @{
 * @addtogroup kernel_drivers_odev Outstream device
 * @{
 *
 * @brief Outstream Device driver
 *
 */

#ifndef _ODEV_H_
#define _ODEV_H_

/**
 * @brief
 *
 * @return int32_t
 */
int32_t init_odev(void);

/**
 * @brief
 *
 * @return int32_t
 */
int32_t start_odev(void);

/**
 * @brief
 *
 * @return int32_t
 */
int32_t stop_odev(void);

/**
 * @brief
 *
 * @return int32_t
 */
int32_t start_odev_stream(void);

/**
 * @brief
 *
 * @return int32_t
 */
int32_t stop_odev_stream(void);

/**
 * @brief
 *
 * @param sAddr
 * @param sLen
 * @return int32_t
 */
int32_t put_to_itab(uint32_t sAddr, uint32_t sLen);

/**
 * @brief
 *
 * @param arg
 * @return int
 */
int32_t odev_irq(void *arg);

/**
 * @brief Set the consume latency object
 *
 * @param lat
 * @return int32_t
 */
int32_t set_consume_latency(uint32_t lat);

/**
 * @brief
 *
 * @return int32_t
 */
int32_t start_consumer(void);

/**
 * @brief
 *
 * @return int32_t
 */
int32_t stop_consumer(void);

/**
 * @brief
 *
 * @return uint32_t
 */
uint32_t run_odev_task(void);

/**
 * @brief Create a odev task object
 *
 */
int32_t create_odev_task(void *arg);
#endif

/**
 * @}
 * @}
 * @}
 *
 */
