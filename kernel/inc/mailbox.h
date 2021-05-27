#ifndef _MAILBOX_H_
#define _MAILBOX_H_

enum letter_type {
	EMPTY = 0,
	TASK_STAT = 1,
	TASK_ODEV = 2,
};

enum letter_status{
	READ = 0,
	NOT_READ = 1,
};

struct mailbox_struct {
	enum letter_status status;
	enum letter_type letter;
};

extern void spin_lock_acquire(volatile uint32_t *pl);
extern void spin_lock_release(volatile uint32_t *pl);
void init_mailbox(void);
enum letter_type pull_mail(void);
void push_mail(enum letter_type letter);
#endif
