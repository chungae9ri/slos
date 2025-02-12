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
 * @brief Printing message to uart0
 *
 */

#ifndef _PRINTK_H_
#define _PRINTK_H_

#include <stdint.h>

/**
 * @brief Printing string to uart0
 *
 * @param [in] fmt Formatted string
 * @param [in] ... Variable arguments list
 */
void printk(const char *fmt, ...);

/**
 * @brief Printing string to buf
 *
 * @param [in] buf Buffer to store string
 * @param [in] fmt Formatted string
 * @param [in] ... Variable argument list
 */
void sprintk(char *buf, const char *fmt, ...);
#endif

/**
 * @}
 * @}
 * @}
 *
 */
