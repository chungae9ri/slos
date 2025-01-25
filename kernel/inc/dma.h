/* SPDX-License-Identifier: MIT OR Apache-2.0
 *
 * Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>
 */

#ifndef _DMA_H_
#define _DMA_H_

#include <stdint.h>

/**
 * @brief Initialize AXI DMA peripheral
 *
 */
int32_t init_dma(void);

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
 * @param arg Pointer to argument
 * @return 0 for success, others for failure
 */
int32_t start_dma(void *arg);

/**
 * @brief DMA irq handler
 *
 * @param arg DMA irq handler argument
 * @return 0 for success, others for failure
 */
int32_t dma_irq(void *arg);

#endif
