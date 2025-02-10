// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

/**
 * @addtogroup kernel
 * @{
 * @addtogroup kernel_core Core
 * @{
 * @addtogroup kernel_core_proc Process management
 * @{
 *
 * @file
 *
 * @brief Fault handler functions
 *
 */

#include <mem_layout.h>
#include <page_table.h>
#include <printk.h>

/** Translation fault in section */
#define TRANSLATION_FLT_SEC 0x5
/** Translation fault in page */
#define TRANSLATION_FLT_PG 0x7
/** System call number for exit */
#define SYS_EXIT 0x0
/** System call number for shell command */
#define SYS_CMD 0x1
/** System call number for write (stdout) */
#define SYS_WRITE 0x2
/** System call number for read (stdin) */
#define SYS_READ 0x3
/** System call number for process sleep */
#define SYS_SLEEP 0x4

/**
 * @brief Exception handler for undefined exception
 *
 */
void platform_undefined_handler(void)
{
	printk("undefined cmd exception!!\n");
	for (;;) {
		;
	}
}

/**
 * @brief Syscall handler for SVC exception
 *
 * @param [in] msg String for stdout
 * @param [in] idx User application ID
 * @param [in] sys_num System call number
 * @return int32_t 0 for success -1 for failure
 */
int32_t platform_syscall_handler(char *msg, int idx, int sys_num)
{
	switch (sys_num) {
	/* syscall exit */
	case SYS_EXIT:
		break;

	/* syscall shellcmd */
	case SYS_CMD:
		break;

	/* syscall write */
	case SYS_WRITE:
		msg = msg + (USER_APP_BASE + USER_APP_GAP * idx);
		printk(msg);
		break;
	/* syscall read */
	case SYS_READ:
		break;

	/* syscall sleep*/
	case SYS_SLEEP:
		break;

	default:
		return -1;
	}

	return 0;
}

/**
 * @brief Prefetch abort handler for
 *
 */
void platform_prefetch_abort_handler(void)
{
	printk("prefetch abort exception!!\n");
	for (;;) {
		;
	}
}

/**
 * @brief Kernel data abort handler
 *
 */
void kernel_abort(void)
{
	printk("data abort exception!!\n");
	for (;;) {
		;
	}
}

/**
 * @brief Data abort handler for data abort exception
 *
 * @param [in] dfsr Data abort status register value
 */
void platform_data_abort_handler(unsigned int dfsr)
{
	/* here is the routine to check the page fault */
	/*unsigned int dfsr;*/
	/* read DFSR */
	/*asm volatile ( "mrc p15, 0, %0, c5, c0, 0" : "=r" (dfsr) ::);*/

	/* section/page fault handler */
	if ((dfsr & TRANSLATION_FLT_PG) == TRANSLATION_FLT_PG) {
		handle_fault();
	} else if ((dfsr & TRANSLATION_FLT_SEC) == TRANSLATION_FLT_SEC) {
		handle_fault();
	} else {
		kernel_abort();
	}
}

/**
 * @}
 * @}
 * @}
 *
 */
