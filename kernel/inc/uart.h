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

/**
 * @brief Initialize uart
 *
 */
void init_uart(void);

/**
 * @brief Send 1 byte to uart output
 *
 * @param c
 */
void poll_out(char c);

/**
 * @brief Receive 1 byte from uart input
 *
 * @return uint8_t
 */
uint8_t poll_in(void);
#endif

/**
 * @}
 * @}
 * @}
 *
 */
