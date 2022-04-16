#include <stdint.h>
#include <xparameters.h>

#define XUARTPS_SR_OFFSET		0x002CU  /**< Channel Status [14:0] */
#define XUARTPS_SR_TXFULL		0x00000010U /**< TX FIFO full */
#define XUARTPS_FIFO_OFFSET		0x0030U  /**< FIFO [7:0] */
#define XUARTPS_SR_RXEMPTY		0x00000002U /**< RX FIFO empty */
#define XUartPs_IsReceiveData(BaseAddress)			 \
	!((Xil_In32((BaseAddress) + XUARTPS_SR_OFFSET) & 	\
				(uint32_t)XUARTPS_SR_RXEMPTY) == (uint32_t)XUARTPS_SR_RXEMPTY)

#define XUartPs_ReadReg(BaseAddress, RegOffset) \
	Xil_In32((BaseAddress) + (uint32_t)(RegOffset))

#define XUartPs_IsTransmitFull(BaseAddress)			 \
	((Xil_In32((BaseAddress) + XUARTPS_SR_OFFSET) & 	\
	  (uint32_t)XUARTPS_SR_TXFULL) == (uint32_t)XUARTPS_SR_TXFULL)

#define XUartPs_WriteReg(BaseAddress, RegOffset, RegisterValue) \
	Xil_Out32((BaseAddress) + (uint32_t)(RegOffset), (uint32_t)(RegisterValue))


static inline void Xil_Out32(uint32_t *Addr, uint32_t Value)
{
	volatile uint32_t *LocalAddr = (volatile uint32_t *)Addr;
	*LocalAddr = Value;
}

static inline uint32_t Xil_In32(uint32_t Addr)
{
	return *(volatile uint32_t *) Addr;
}

void XUartPs_SendByte(uint32_t BaseAddress, uint8_t Data)
{
	/* Wait until there is space in TX FIFO */
	while (XUartPs_IsTransmitFull(BaseAddress)) {
		;
	}

	/* Write the byte into the TX FIFO */
	XUartPs_WriteReg(BaseAddress, XUARTPS_FIFO_OFFSET, (uint32_t)Data);
}

void outbyte(char c) {
	XUartPs_SendByte(STDOUT_BASEADDRESS, c);
}

uint8_t XUartPs_RecvByte(uint32_t BaseAddress)
{
	uint32_t RecievedByte;
	/* Wait until there is data */
	while (!XUartPs_IsReceiveData(BaseAddress)) {
		;
	}
	RecievedByte = XUartPs_ReadReg(BaseAddress, XUARTPS_FIFO_OFFSET);
	/* Return the byte received */
	return (uint8_t)RecievedByte;
}

uint8_t inbyte(void) 
{
	return XUartPs_RecvByte(STDIN_BASEADDRESS);
}
