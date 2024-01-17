#include <stdint.h>

#include <uart.h>
#include <printk.h>

#define ZYNQMP_UART0_BASEADDR		0x00FF000000
#define ZYNQMP_UART1_BASEADDR		0x00FF010000

int main(void)
{
	init_uart(ZYNQMP_UART1_BASEADDR);

	printk("hello world\n");

	return 0;
}
