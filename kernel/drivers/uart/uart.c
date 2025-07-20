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

#include <error.h>
#include <regops.h>
#include <uart.h>

#include <generated_devicetree_defs.h>

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
DEVICE_DEFINE_IDX(uart, 0);
#else
DEVICE_DEFINE_IDX(uart, 0);
#endif

int32_t poll_out(struct device *dev, char c)
{
	uint32_t reg_val;

	reg_val = read32(dev->base_addr + SR_OFFSET);

	while ((reg_val & BM_SR_TXFULL) == BM_SR_TXFULL) {
		reg_val = read32(dev->base_addr + SR_OFFSET);
	}

	write32(dev->base_addr + FIFO_OFFSET, (uint32_t)c);

	reg_val = read32(dev->base_addr + SR_OFFSET);

	while ((reg_val & BM_SR_TXFULL) == BM_SR_TXFULL) {
		reg_val = read32(dev->base_addr + SR_OFFSET);
	}

	return 0;
}

uint8_t poll_in(struct device *dev)
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
	while ((read32(dev->base_addr + SR_OFFSET) & BM_SR_RXEMPTY) ==
	       BM_SR_RXEMPTY) {
		;
	}
#endif

	c = read32(dev->base_addr + FIFO_OFFSET);

	return (uint8_t)c;
}

int32_t init_uart(struct device *dev)
{
	uint32_t idx;

	idx = dev->idx;

	if (idx == 0) {
		dev->name = DT_GET_COMPAT(0);
		dev->base_addr = DT_GET_BASE_ADDR(0);
#ifdef AARCH32
		dev->irq = DT_GET_IRQ(0);
#endif

	} else {
		return -EINVAL;
	}

	return 0;
}

int32_t configure_uart(struct device *dev)
{
	/* Set 115200 8N1 */
	write32(dev->base_addr + CR_OFFSET, 0x00000114);
	write32(dev->base_addr + MR_OFFSET, 0x00000020);
	write32(dev->base_addr + IER_OFFSET, 0x00000000);
	write32(dev->base_addr + IDR_OFFSET, 0x00000000);
	write32(dev->base_addr + BAUDGEN_OFFSET, 0x0000007C);
	write32(dev->base_addr + RXTOUT_OFFSET, 0x0000000A);
	write32(dev->base_addr + RXWM_OFFSET, 0x00000038);
	write32(dev->base_addr + MODEMCR_OFFSET, 0x00000003);
	write32(dev->base_addr + BAUD_DIV_OFFSET, 0x00000006);
	write32(dev->base_addr + FLOW_DELAY_OFFSET, 0x00000000);
	write32(dev->base_addr + TX_FIFO_TRIG_LV_OFFSET, 0x00000020);

	return 0;
}

/**
 * @}
 * @}
 * @}
 *
 */
