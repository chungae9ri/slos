#include <stdint.h>

#define ZYNQMP_UART1_BASEADDR		0x00FF010000

typedef struct uart_ps {
	volatile uint32_t CR;
	volatile uint32_t MR;
	volatile uint32_t IER;
	volatile uint32_t IDR;
	volatile const uint32_t IMR;
	volatile uint32_t ISR;
	volatile uint32_t BAUDGEN;
	volatile uint32_t RXTOUT;
	volatile uint32_t RXWM;
	volatile uint32_t MODEMCR;
	volatile uint32_t MODEMSR;
	volatile const uint32_t SR;
	volatile uint32_t FIFO;
	volatile uint32_t BAUD_DIV;
	volatile uint32_t FLOW_DELAY;
	volatile uint32_t TX_FIFO_TRIG_LV;
} UART_PS_t;

static void init_uart(void)
{
	UART_PS_t *puart = (UART_PS_t *)(ZYNQMP_UART1_BASEADDR);
	puart->CR = 0x00000114;
	puart->MR = 0x00000020;
	puart->IER = 0x00000000;
	puart->IDR = 0x00000000;
	puart->BAUDGEN = 0x0000007C;
	puart->RXTOUT = 0x0000000A;
	puart->RXWM = 0x00000038;
	puart->MODEMCR = 0x00000003;
	puart->BAUD_DIV = 0x00000006;
	puart->FLOW_DELAY = 0x00000000;
	puart->TX_FIFO_TRIG_LV = 0x00000020;
}


int main(void)
{
	init_uart();
	/*printk("Hello World!\n");*/

	return 0;
}
