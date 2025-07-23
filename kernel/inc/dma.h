/* SPDX-License-Identifier: MIT OR Apache-2.0
 *
 * Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>
 */

/**
 * @addtogroup kernel
 * @{
 * @addtogroup kernel_drivers Drivers
 * @{
 * @addtogroup kernel_drivers_dma DMA driver
 * @{
 *
 * @brief Driver for DMA peripheral implemented in FPGA
 *
 */

#ifndef _DMA_H_
#define _DMA_H_

#include <stdint.h>

#include <device.h>

DEVICE_DECLARE_IDX(dma, 0);
/**
 * @brief Initialize AXI DMA peripheral
 *
 * @param dev DMA device controller instance
 */
int32_t init_dma(struct device *dev);

/**
 * @brief Set the DMA work
 *
 * @param src Source address of memory for DMA
 * @param dst Destination address of memory for DMA
 * @param len Byte length of DMA transfer
 *
 * @return 0 for success, others for failure
 */
int32_t set_dma_work(uint32_t src, uint32_t dst, uint32_t len);

/**
 * @brief Start DMA transfer
 *
 * @param dev DMA device controller instance
 * @return 0 for success, others for failure
 */
int32_t start_dma(struct device *dev);

/**
 * @brief DMA irq handler
 *
 * @param arg DMA irq handler argument
 * @return 0 for success, others for failure
 */
int32_t dma_irq(void *arg);

#endif

/**
 * @}
 * @}
 * @}
 *
 */
