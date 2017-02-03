#include <uart_dm.h>
#include <locks.h>

extern void enable_interrupt();
extern void disable_interrupt();
extern void spin_lock_acquire(volatile uint32_t *pl);
extern void spin_lock_release(volatile uint32_t *pl);
volatile uint32_t uartlock;

int print_msg(const char *str)
{
	spin_lock_acquire_irqsafe(&uartlock);
	/*spin_lock_acquire(&uartlock);*/
	/*disable_interrupt();*/
	while (*str != '\0') {
		uart_putc(0, *str++);
	}
	/*enable_interrupt();*/
	/*spin_lock_release(&uartlock);*/
	spin_lock_release_irqsafe(&uartlock);
	return 0;
}

