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

#include <device.h>

DEVICE_DECLARE_IDX(odev, 0);

/**
 * @brief Initialize the Outstream device
 *
 * @param dev Outstream device instance
 *
 * @return int32_t
 */
int32_t init_odev(struct device *dev);

/**
 * @brief odev irq handler
 *
 * @param arg Outstream device instance
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
 * @brief Start the consumer device in Outstream datapath
 *
 * This function start the consumer device which drains data from
 * the FIFOs in the outstream datapath.
 *
 * @param dev Outstream device instance
 * @return int32_t
 */
int32_t start_consumer(struct device *dev);

/**
 * @brief Stop the consumer device in Outstream datapath
 *
 * This function stops the consumer device which fills data into
 * the FIFOs in the outstream datapath. When data reaches the FIFO
 * threshold, data isn't copied to the FIFO to prevent overflow.
 *
 * @param dev Outstream device instance
 * @return int32_t
 */
int32_t stop_consumer(struct device *dev);

/**
 * @brief Run Outstream device driver task
 *
 * @return uint32_t
 */
uint32_t run_odev_task(void);

/**
 * @brief Create a odev task object
 *
 * @return uint32_t
 */
int32_t create_odev_task(void *arg);

#endif

/**
 * @}
 * @}
 * @}
 *
 */
