// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

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
 * @brief Mailbox used for coummunicating between CPU 0 and CPU 1
 *
 */

#include <stdint.h>
#include <mailbox.h>
#include <ops.h>
#include <defs.h>

/** Mailbox for CPU 0 */
__section(".mailbox") struct mailbox_struct mailbox_0;
/** Mailbox for CPU 1 */
__section(".mailbox") struct mailbox_struct mailbox_1;

/** Spinlock synchronization */
uint32_t mailbox_lock;

void init_mailbox(void)
{
	mailbox_0.status = READ;
	mailbox_0.letter = EMPTY;
	mailbox_1.status = READ;
	mailbox_1.letter = EMPTY;
}

void push_mail(enum letter_type letter)
{
	uint32_t cpuid;
	struct mailbox_struct *pmailbox;

	/* Get current cpuid and
	 * set target cpuid
	 */
	cpuid = smp_processor_id();
	spin_lock_acquire(&mailbox_lock);

	if (cpuid == 0) {
		pmailbox = &mailbox_1;
	} else {
		pmailbox = &mailbox_0;
	}

	/* Blocking until the letter is read */
	while (1) {
		if (pmailbox->status == READ) {
			pmailbox->letter = letter;
			pmailbox->status = NOT_READ;
			spin_lock_release(&mailbox_lock);
			break;
		}
	}
}

enum letter_type pull_mail(void)
{
	uint32_t cpuid = smp_processor_id();
	enum letter_type letter;
	struct mailbox_struct *pmailbox;

	if (cpuid == 0) {
		pmailbox = &mailbox_0;
	} else {
		pmailbox = &mailbox_1;
	}

	spin_lock_acquire(&mailbox_lock);
	if (pmailbox->status == READ) {
		letter = EMPTY;
	} else {
		letter = pmailbox->letter;
		pmailbox->status = READ;
	}

	spin_lock_release(&mailbox_lock);

	return letter;
}

/**
 * @}
 * @}
 * @}
 *
 */
