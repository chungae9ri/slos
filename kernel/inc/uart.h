/* SPDX-License-Identifier: MIT OR Apache-2.0
 *
 * Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>
 */

/**
 * @addtogroup kernel
 * @{
 * @addtogroup kernel_drivers Drivers
 * @{
 * @addtogroup kernel_drivers_uart Uart driver
 * @{
 *
 * @file
 *
 * @brief Driver for Uart peripheral
 *
 */

#ifndef _UART_H_
#define _UART_H_

#include <device.h>

DEVICE_DECLARE_IDX(uart, 0);
/**
 * @brief Initialize uart
 *
 * @param dev Uart device instance
 */
int32_t init_uart(struct device *dev);

/**
 * @brief Configure uart device
 * 
 * @param dev Uart device instance
 * @retval 0 for success others for failure
 */
int32_t configure_uart(struct device *dev);

/**
 * @brief Send 1 byte to uart output
 *
 * @param dev Uart device instance
 * @param c 1byte data to be sent
 * @retval 0 for success others for failure
 */
int32_t poll_out(struct device *dev, char c);

/**
 * @brief Receive 1 byte from uart input
 *
 * @param dev Uart device instance
 * @retval 0 for success others for failure
 */
uint8_t poll_in(struct device *dev);
#endif

/**
 * @}
 * @}
 * @}
 *
 */
