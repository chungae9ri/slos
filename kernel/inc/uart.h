// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

#ifndef _UART_H_
#define _UART_H_

/**
 * @brief 
 * 
 */
void init_uart(void);

/**
 * @brief 
 * 
 * @param c 
 */
void poll_out(char c);

/**
 * @brief 
 * 
 * @return uint8_t 
 */
uint8_t poll_in(void);
#endif
