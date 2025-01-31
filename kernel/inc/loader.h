/* SPDX-License-Identifier: MIT OR Apache-2.0
 *
 * Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>
 */

#ifndef __LOADER_H__
#define __LOADER_H__
#include <fs.h>

/** Max user task number */
#define MAX_USR_TASK 1

extern struct task_struct *upt[MAX_USR_TASK];

int32_t load_ramdisk_app(FILE_SYSTEM_TYPE fs_t, uint32_t app_idx);
void exit_elf(uint32_t idx);
#endif
