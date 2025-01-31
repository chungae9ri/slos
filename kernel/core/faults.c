// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

#include <mem_layout.h>
#include <page_table.h>
#include <printk.h>

#define ALIGNMENT_FLT	    0x1
#define BUS_ERR_TRN_LVL1    0xc
#define BUS_ERR_TRN_LVL2    0xe
#define TRANSLATION_FLT_SEC 0x5
#define TRANSLATION_FLT_PG  0x7
#define DOMAIN_FLT_SEC	    0x9
#define DOMAIN_FLT_PG	    0xb
#define PERM_FLT_SEC	    0xd
#define PERM_FLT_PG	    0xf
#define BUS_ERR_LF_SEC	    0x4
#define BUS_ERR_LF_PG	    0x6
#define BUS_ERR_OTH_SEC	    0x8
#define BUS_ERR_OTH_PG	    0xa

#define SYS_EXIT  0x0
#define SYS_CMD	  0x1
#define SYS_WRITE 0x2
#define SYS_READ  0x3
#define SYS_SLEEP 0x4

void platform_undefined_handler(void)
{
	printk("undefined cmd exception!!\n");
	for (;;) {
		;
	}
}

int platform_syscall_handler(char *msg, int idx, int sys_num)
{
	int ret = 0;

	switch (sys_num) {
	/* syscall exit */
	case SYS_EXIT:
		break;

	/* syscal shellcmd */
	case SYS_CMD:
		break;

	/* syscal write */
	case SYS_WRITE:
		msg = msg + (USER_APP_BASE + USER_APP_GAP * idx);
		printk(msg);
		break;
	/* syscal read */
	case SYS_READ:
		break;

	/* syscal sleep*/
	case SYS_SLEEP:
		break;

	default:
		break;
	}
	return ret;
}

void platform_prefetch_abort_handler(void)
{
	printk("prefetch abort exception!!\n");
	for (;;) {
		;
	}
}

void kernel_abort(void)
{
	printk("data abort exception!!\n");
	for (;;) {
		;
	}
}

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
