#ifndef _LOCKS_H_
#define _LOCKS_H_
void spin_lock_acquire_irqsafe(volatile uint32_t *lock);
void spin_lock_release_irqsafe(volatile uint32_t *lock);
inline int enter_critical_section(void);
inline int exit_critical_section(void);
#endif
