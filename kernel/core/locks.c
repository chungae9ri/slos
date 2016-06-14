#include <stdint-gcc.h>

extern void spin_lock_acquire(volatile uint32_t *lock);
extern void spin_lock_release(volatile uint32_t *lock);
extern void enable_interrupt();
extern void disable_interrupt();
extern int get_svc();
uint32_t critical_section_count = 0;

#define MODE_SVC	0x13

void spin_lock_acquire_irqsafe(volatile uint32_t *lock)
{
	disable_interrupt();
	spin_lock_acquire(lock);
}

void spin_lock_release_irqsafe(volatile uint32_t *lock)
{
	int svc;
	spin_lock_release(lock);
	svc = get_svc();
	if(svc == MODE_SVC) enable_interrupt();
}

/* critical section function for UP*/
inline void enter_critical_section()
{
	critical_section_count++;
	if (critical_section_count == 1) {
		disable_interrupt();
	}
}

inline void exit_critical_section()
{
	critical_section_count--;
	if (critical_section_count == 0) {
		enable_interrupt();
	}
}
