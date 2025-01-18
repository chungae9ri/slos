// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

#ifndef _DEFS_H_
#define _DEFS_H_

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define container_of(ptr, type, member) ((type *)((unsigned int)ptr - offsetof(type, member)))
#endif
