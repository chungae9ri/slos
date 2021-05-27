#include <stdint.h>
#include <mailbox.h>

__attribute__((__section__(".mailbox"))) 
		struct mailbox_struct mailbox_0;
__attribute__((__section__(".mailbox")))
		struct mailbox_struct mailbox_1;


volatile uint32_t mailbox_lock = 0;
uint32_t smp_processor_id(void);

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

	/* get current cpuid and 
	 * set target cpuid
	 */
	cpuid = smp_processor_id();
	if (cpuid == 0) {
		pmailbox = &mailbox_1;
	} else {
		pmailbox = &mailbox_0;
	}

	// blocking until the letter is read
	while (1) {
		spin_lock_acquire(&mailbox_lock);
		if (pmailbox->status == READ) {
			pmailbox->letter = letter;
			pmailbox->status = NOT_READ;
			spin_lock_release(&mailbox_lock);
			break;
		} else {
			spin_lock_release(&mailbox_lock);
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
		spin_lock_release(&mailbox_lock);
		return EMPTY;
	} else {
		letter = pmailbox->letter;
		pmailbox->status = READ;
		spin_lock_release(&mailbox_lock);
		return letter;
	}
}
