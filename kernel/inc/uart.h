// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

#ifndef _UART_H_
#define _UART_H_
void init_uart(uint32_t uart_base_addr);
void outbyte(char c);
uint8_t inbyte(void);
#endif
