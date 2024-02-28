// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

#ifndef _PRINTK_H_
#define _PRINTK_H_
#include <stdint.h>

void printk(const char *fmt, ...);
void sprintk(char *buf, const char *fmt, ...);
#endif
