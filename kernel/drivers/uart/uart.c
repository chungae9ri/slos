#include <stdint.h>
/*#include <xparameters.h>*/

#define CR_OFFSET 0x0
#define MR_OFFSET 0x4
#define IER_OFFSET 0x8
#define IDR_OFFSET 0xC
#define IMR_OFFSET 0x10
#define ISR_OFFSET 0x14
#define BAUDGEN_OFFSET 0x18
#define RXTOUT_OFFSET 0x1C
#define RXWM_OFFSET 0x20
#define MODEMCR_OFFSET 0x24
#define MODEMSR_OFFSET 0x28
#define SR_OFFSET 0x2C
#define FIFO_OFFSET 0x30
#define BAUD_DIV_OFFSET 0x34
#define FLOW_DELAY_OFFSET 0x38
#define TX_FIFO_TRIG_LV_OFFSET 0x44
#define RX_FIFO_TRIG_LV_OFFSET 0x48

#define BM_SR_TXFULL		0x00000010U /**< TX FIFO full */
#define BM_SR_RXEMPTY		0x00000002U /**< RX FIFO empty */

static uint32_t uart_base_addr;

#define XUartPs_IsReceiveData()			 \
	!((Xil_In32((uart_base_addr) + SR_OFFSET) & 	\
				(uint32_t)BM_SR_RXEMPTY) == (uint32_t)BM_SR_RXEMPTY)

#define XUartPs_ReadReg(RegOffset) \
	Xil_In32((uart_base_addr) + (uint32_t)(RegOffset))

#define XUartPs_IsTransmitFull()			 \
	((Xil_In32((uart_base_addr) + SR_OFFSET) & 	\
	  (uint32_t)BM_SR_TXFULL) == (uint32_t)BM_SR_TXFULL)

#define XUartPs_WriteReg(RegOffset, RegisterValue) \
	Xil_Out32((uint32_t *)((uart_base_addr) + (uint32_t)(RegOffset)), (uint32_t)(RegisterValue))


static inline void Xil_Out32(uint32_t *Addr, uint32_t Value)
{
	volatile uint32_t *LocalAddr = (volatile uint32_t *)Addr;
	*LocalAddr = Value;
}

static inline uint32_t Xil_In32(uint32_t Addr)
{
	return *(volatile uint32_t *) Addr;
}

void XUartPs_SendByte(uint8_t Data)
{
	/* Wait until there is space in TX FIFO */
	while (XUartPs_IsTransmitFull()) {
		;
	}

	/* Write the byte into the TX FIFO */
	XUartPs_WriteReg(FIFO_OFFSET, (uint32_t)Data);
}

void outbyte(char c) {
	XUartPs_SendByte(c);
}

uint8_t XUartPs_RecvByte(void)
{
	uint32_t RecievedByte;
	/* Wait until there is data */
	while (!XUartPs_IsReceiveData()) {
		;
	}
	RecievedByte = XUartPs_ReadReg(FIFO_OFFSET);
	/* Return the byte received */
	return (uint8_t)RecievedByte;
}

uint8_t inbyte() 
{
	return XUartPs_RecvByte();
}

void init_uart(uint32_t base_addr)
{
	uart_base_addr = base_addr;

	*((uint32_t *)(uart_base_addr + CR_OFFSET)) = 0x00000114;
	*((uint32_t *)(uart_base_addr + MR_OFFSET)) = 0x00000020;
	*((uint32_t *)(uart_base_addr + IER_OFFSET)) = 0x00000000;
	*((uint32_t *)(uart_base_addr + IDR_OFFSET)) = 0x00000000;
	*((uint32_t *)(uart_base_addr + BAUDGEN_OFFSET)) = 0x0000007C;
	*((uint32_t *)(uart_base_addr + RXTOUT_OFFSET)) = 0x0000000A;
	*((uint32_t *)(uart_base_addr + RXWM_OFFSET)) = 0x00000038;
	*((uint32_t *)(uart_base_addr + MODEMCR_OFFSET)) = 0x00000003;
	*((uint32_t *)(uart_base_addr + BAUD_DIV_OFFSET)) = 0x00000006;
	*((uint32_t *)(uart_base_addr + FLOW_DELAY_OFFSET)) = 0x00000000;
	*((uint32_t *)(uart_base_addr + TX_FIFO_TRIG_LV_OFFSET)) = 0x00000020;
}

