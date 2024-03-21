// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

#include <stdint.h>

/**
 * @brief 
 * 
 */
void init_dma(void);

/**
 * @brief Set the dma work object
 * 
 * @param src 
 * @param dst 
 * @param len 
 */
void set_dma_work(uint32_t src, uint32_t dst, uint32_t len);

/**
 * @brief 
 * 
 * @param arg 
 */
void start_dma(void *arg);

/**
 * @brief 
 * 
 * @param arg 
 * @return int 
 */
int dma_irq (void *arg);
