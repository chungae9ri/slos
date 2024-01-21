#include <stdint.h>

#include <uart.h>
#include <printk.h>

#define ZYNQMP_UART0_BASEADDR		0x00FF000000
#define ZYNQMP_UART1_BASEADDR		0x00FF010000

int main(void)
{
	uint32_t current_el= 0;
	init_uart(ZYNQMP_UART1_BASEADDR);

	/* read currentEL*/
	asm volatile ("mrs %[cel], CurrentEL" : [cel] "=r" (current_el)::);
	printk("currentEL: %d\n", current_el); 

	return 0;
}
