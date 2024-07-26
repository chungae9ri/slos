// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

#include <gic_390.h>
#include <sgi.h>
#include <sched.h>
#include <task.h>
#include <odev.h>
#include <printk.h>
#include <mailbox.h>

void enable_sgi_irq(int vec, int (sgi_irq_handler)(void *arg))
{
	gic_register_int_handler(vec, sgi_irq_handler, NULL);
}

int sgi_irq(void *arg)
{
	/* don't enable this message */
#if 0
	struct sgi_data *pdat;

	pdat = (struct sgi_data *)arg;
	printk("sgi intr 0x%x from cpu: 0x%x\n", pdat->num, pdat->cpuid);
#endif

	enum letter_type letter = pull_mail();
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
