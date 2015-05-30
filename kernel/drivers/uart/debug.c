#include <uart_dm.h>
#include <locks.h>

volatile uint32_t uartlock;

int print_msg(const char *str)
{
	spin_lock_acquire_irqsafe(&uartlock);
	while (*str != 0) {
		uart_putc(0, *str++);
	}
	spin_lock_release_irqsafe(&uartlock);
	return 0;
}

