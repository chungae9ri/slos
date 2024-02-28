// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

#define REG32(addr) 	((volatile unsigned int *)(addr))

#define writel(v, a) 	((*REG32(a)) = (v))
#define readl(a)	(*REG32(a))
