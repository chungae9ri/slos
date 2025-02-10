// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

/**
 * @addtogroup kernel
 * @{
 * @addtogroup kernel_drivers Drivers
 * @{
 * @addtogroup kernel_drivers_uart Uart driver
 * @{
 *
 * @file
 *
 * @brief Driver for Uart peripheral
 *
 */

/** Uart compatible string in devicetree */
#define DEVICE_DT_COMPAT PS_UART

#include <stdint.h>

#include <regops.h>

#include <generated_devicetree_defs.h>
#include <device.h>

#define CR_OFFSET	       0x0
#define MR_OFFSET	       0x4
#define IER_OFFSET	       0x8
#define IDR_OFFSET	       0xC
#define IMR_OFFSET	       0x10
#define ISR_OFFSET	       0x14
#define BAUDGEN_OFFSET	       0x18
#define RXTOUT_OFFSET	       0x1C
#define RXWM_OFFSET	       0x20
#define MODEMCR_OFFSET	       0x24
#define MODEMSR_OFFSET	       0x28
#define SR_OFFSET	       0x2C
#define FIFO_OFFSET	       0x30
#define BAUD_DIV_OFFSET	       0x34
#define FLOW_DELAY_OFFSET      0x38
#define TX_FIFO_TRIG_LV_OFFSET 0x44
#define RX_FIFO_TRIG_LV_OFFSET 0x48

#define BM_SR_TXFULL  0x00000010U /**< TX FIFO full */
#define BM_SR_RXEMPTY 0x00000002U /**< RX FIFO empty */

#ifdef AARCH32
DEVICE_DEFINE(uart_0, DT_GET_COMPAT(0), DT_GET_BASE_ADDR(0), DT_GET_IRQ(0));
#else
DEVICE_DEFINE(uart_0, DT_GET_COMPAT(0), DT_GET_BASE_ADDR(0), 0);
#endif

void poll_out(char c)
{
	uint32_t reg_val;

	reg_val = read32(DEVICE_GET_BASE_ADDR(uart_0) + SR_OFFSET);

	while ((reg_val & BM_SR_TXFULL) == BM_SR_TXFULL) {
		reg_val = read32(DEVICE_GET_BASE_ADDR(uart_0) + SR_OFFSET);
	}

	write32(DEVICE_GET_BASE_ADDR(uart_0) + FIFO_OFFSET, (uint32_t)c);

	reg_val = read32(DEVICE_GET_BASE_ADDR(uart_0) + SR_OFFSET);

	while ((reg_val & BM_SR_TXFULL) == BM_SR_TXFULL) {
		reg_val = read32(DEVICE_GET_BASE_ADDR(uart_0) + SR_OFFSET);
	}
}

uint8_t poll_in(void)
{
	uint32_t c = 0;

	/* Wait until there is data */
#if 0
	/* FIXME: 
	 * This routine doesn't work. while loop exits 
	 * even though there is no input char. Could be because 
	 * while loop is preempted and when gets back the while loop
	 * exits because of corrupted context switching.
	 */
	uint32_t reg_val;

	reg_val = read32(DEVICE_GET_BASE_ADDR(uart_0) + SR_OFFSET);
	while ((reg_val & BM_SR_RXEMPTY) == BM_SR_RXEMPTY) {
		reg_val = read32(DEVICE_GET_BASE_ADDR(uart_0) + SR_OFFSET);
	}
#else
	while ((read32(DEVICE_GET_BASE_ADDR(uart_0) + SR_OFFSET) & BM_SR_RXEMPTY) ==
	       BM_SR_RXEMPTY) {
		;
	}
#endif

	c = read32(DEVICE_GET_BASE_ADDR(uart_0) + FIFO_OFFSET);

	return (uint8_t)c;
}

void init_uart(void)
{
	write32(DEVICE_GET_BASE_ADDR(uart_0) + CR_OFFSET, 0x00000114);
	write32(DEVICE_GET_BASE_ADDR(uart_0) + MR_OFFSET, 0x00000020);
	write32(DEVICE_GET_BASE_ADDR(uart_0) + IER_OFFSET, 0x00000000);
	write32(DEVICE_GET_BASE_ADDR(uart_0) + IDR_OFFSET, 0x00000000);
	write32(DEVICE_GET_BASE_ADDR(uart_0) + BAUDGEN_OFFSET, 0x0000007C);
	write32(DEVICE_GET_BASE_ADDR(uart_0) + RXTOUT_OFFSET, 0x0000000A);
	write32(DEVICE_GET_BASE_ADDR(uart_0) + RXWM_OFFSET, 0x00000038);
	write32(DEVICE_GET_BASE_ADDR(uart_0) + MODEMCR_OFFSET, 0x00000003);
	write32(DEVICE_GET_BASE_ADDR(uart_0) + BAUD_DIV_OFFSET, 0x00000006);
	write32(DEVICE_GET_BASE_ADDR(uart_0) + FLOW_DELAY_OFFSET, 0x00000000);
	write32(DEVICE_GET_BASE_ADDR(uart_0) + TX_FIFO_TRIG_LV_OFFSET, 0x00000020);
}

/**
 * @}
 * @}
 * @}
 *
 */
