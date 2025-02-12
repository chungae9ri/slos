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
 * @brief Mailbox functions
 *
 */

#ifndef _MAILBOX_H_
#define _MAILBOX_H_

/** Letter type */
enum letter_type {
	EMPTY = 0,     /**< Mailbox empty */
	TASK_STAT = 1, /**< Letter for task_stat command */
	TASK_ODEV = 2, /**< Letter for odev command */
};

/** Letter in mailbox status */
enum letter_status {
	READ = 0,     /**< Letter is read */
	NOT_READ = 1, /**< Letter isn't yet read */
};

/** Mailbox struct */
struct mailbox_struct {
	enum letter_status status; /**< Mailbox status */
	enum letter_type letter;   /**< Letter type */
};

/**
 * @brief Initialize mailbox
 *
 */
void init_mailbox(void);

/**
 * @brief Pull letter from mailbox
 *
 * @return enum letter_type
 */
enum letter_type pull_mail(void);

/**
 * @brief Push letter to mailbox
 *
 * @param [in] letter Letter to be pushed to mailbox
 */
void push_mail(enum letter_type letter);

#endif

/**
 * @}
 * @}
 * @}
 *
 */
