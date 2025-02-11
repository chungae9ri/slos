/* SPDX-License-Identifier: MIT OR Apache-2.0
 *
 * Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>
 */
/**
 * @addtogroup kernel
 * @{
 * @addtogroup kernel_core Core
 * @{
 * @addtogroup kernel_core_misc Miscellaneous kernel modules
 * @{
 *
 * @file
 *
 * @brief ELF loader functions
 *
 */

#ifndef __LOADER_H__
#define __LOADER_H__

#include <fs.h>

/** Max user task number */
#define MAX_USR_TASK 1

/** Task container for user tasks */
extern struct task_struct *upt[MAX_USR_TASK];

/**
 * @brief Load application from the ramdisk
 *
 * @param [in] fs_t File system type
 * @param [in] app_idx Application index in the ramdisk
 * @return int32_t 0 for success
 */
int32_t load_ramdisk_app(FILE_SYSTEM_TYPE fs_t, uint32_t app_idx);

#endif

/**
 * @}
 * @}
 * @}
 *
 */
