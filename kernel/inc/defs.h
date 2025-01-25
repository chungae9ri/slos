/* SPDX-License-Identifier: MIT OR Apache-2.0
 *
 * Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>
 */

#ifndef _DEFS_H_
#define _DEFS_H_

/** Calculate the base address of a structure */
#define container_of(ptr, type, member) ((type *)((unsigned int)ptr - offsetof(type, member)))

/** Section attribute macro */
#define __section(x) __attribute__((__section__(x)))

#endif
