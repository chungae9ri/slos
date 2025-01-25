// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

#include <gic_v1.h>
#include <sgi.h>
#include <sched.h>
#include <task.h>
#include <odev.h>
#include <printk.h>
#include <mailbox.h>

void enable_sgi_irq(int vec, int32_t(sgi_irq_handler)(void *arg))
{
	gic_register_int_handler(vec, sgi_irq_handler, NULL);
}

int32_t sgi_irq(void *arg)
{
	enum letter_type letter = pull_mail();

	/* Don't print debug message here */

	switch (letter) {
	case EMPTY:
		break;

	case TASK_STAT:
		enqueue_workq(print_task_stat, NULL);
		wakeup_workq_worker();
		break;

	case TASK_ODEV:
		enqueue_workq(create_odev_task, NULL);
		wakeup_workq_worker();
		break;

	default:
		break;
	}

	return 0;
}
