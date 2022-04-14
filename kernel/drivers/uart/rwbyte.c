#include <stdint.h>
#include "xparameters.h"

#define XUARTPS_SR_OFFSET		0x002CU  /**< Channel Status [14:0] */
#define XUARTPS_SR_TXFULL		0x00000010U /**< TX FIFO full */
#define XUARTPS_FIFO_OFFSET		0x0030U  /**< FIFO [7:0] */
#define XUARTPS_SR_RXEMPTY		0x00000002U /**< RX FIFO empty */


/*****************************************************************************/
/**
*
* Performs an output operation for a 32-bit memory location by writing the
* specified Value to the the specified address.
*
* @param	Addr contains the address to perform the output operation
*		at.
* @param	Value contains the Value to be output at the specified address.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static inline void Xil_Out32(uint32_t *Addr, uint32_t Value)
{
	volatile uint32_t *LocalAddr = (volatile uint32_t *)Addr;
	*LocalAddr = Value;
}

/*****************************************************************************/
/**
*
* Performs an input operation for a 32-bit memory location by reading from the
* specified address and returning the Value read from that address.
*
* @param	Addr contains the address to perform the input operation
*		at.
*
* @return	The Value read from the specified input address.
*
* @note		None.
*
******************************************************************************/
static inline uint32_t Xil_In32(uint32_t Addr)
{
	return *(volatile uint32_t *) Addr;
}

/****************************************************************************/
/**
* Determine if a byte of data can be sent with the transmitter.
*
* @param	BaseAddress contains the base address of the device.
*
* @return	TRUE if the TX FIFO is full, FALSE if a byte can be put in the
*		FIFO.
*
* @note		C-Style signature:
*		uint32_t XUartPs_IsTransmitFull(uint32_t BaseAddress)
*
******************************************************************************/
#define XUartPs_IsTransmitFull(BaseAddress)			 \
	((Xil_In32((BaseAddress) + XUARTPS_SR_OFFSET) & 	\
	 (uint32_t)XUARTPS_SR_TXFULL) == (uint32_t)XUARTPS_SR_TXFULL)

/***************************************************************************/
/**
* Write a UART register.
*
* @param	BaseAddress contains the base address of the device.
* @param	RegOffset contains the offset from the base address of the
*		device.
* @param	RegisterValue is the value to be written to the register.
*
* @return	None.
*
* @note		C-Style signature:
*		void XUartPs_WriteReg(uint32_t BaseAddress, int RegOffset,
*						   u16 RegisterValue)
*
******************************************************************************/
#define XUartPs_WriteReg(BaseAddress, RegOffset, RegisterValue) \
	Xil_Out32((BaseAddress) + (uint32_t)(RegOffset), (uint32_t)(RegisterValue))

/****************************************************************************/
/**
*
* This function sends one byte using the device. This function operates in
* polled mode and blocks until the data has been put into the TX FIFO register.
*
* @param	BaseAddress contains the base address of the device.
* @param	Data contains the byte to be sent.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
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
/****************************************************************************/
/**
* Determine if there is receive data in the receiver and/or FIFO.
*
* @param	BaseAddress contains the base address of the device.
*
* @return	TRUE if there is receive data, FALSE otherwise.
*
* @note		C-Style signature:
*		uint32_t XUartPs_IsReceiveData(uint32_t BaseAddress)
*
******************************************************************************/
#define XUartPs_IsReceiveData(BaseAddress)			 \
	!((Xil_In32((BaseAddress) + XUARTPS_SR_OFFSET) & 	\
	(uint32_t)XUARTPS_SR_RXEMPTY) == (uint32_t)XUARTPS_SR_RXEMPTY)

/****************************************************************************/
/**
* Read a UART register.
*
* @param	BaseAddress contains the base address of the device.
* @param	RegOffset contains the offset from the base address of the
*		device.
*
* @return	The value read from the register.
*
* @note		C-Style signature:
*		uint32_t XUartPs_ReadReg(uint32_t BaseAddress, int RegOffset)
*
******************************************************************************/
#define XUartPs_ReadReg(BaseAddress, RegOffset) \
	Xil_In32((BaseAddress) + (uint32_t)(RegOffset))

/****************************************************************************/
/**
*
* This function receives a byte from the device. It operates in polled mode
* and blocks until a byte has received.
*
* @param	BaseAddress contains the base address of the device.
*
* @return	The data byte received.
*
* @note		None.
*
*****************************************************************************/
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
