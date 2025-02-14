/* SPDX-License-Identifier: MIT OR Apache-2.0
 *
 * Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>
 */

/**
 * @addtogroup kernel
 * @{
 * @addtogroup kernel_core Core
 * @{
 * @addtogroup kernel_core_proc Process management
 * @{
 *
 * @brief Kernel SGI (Software Generated Interrupt) modlue
 *
 */

#ifndef _SGI_H_
#define _SGI_H_

#include <stdint.h>

/** SGI interrupt handler argument data */
struct sgi_data {
	uint32_t num;	/**< SGI number */
	uint32_t cpuid; /**< CPU ID */
};

/**
 * @brief SGI interrupt handler
 *
 * @param [in] arg SGI interrupt handler agrument
 * @return int32_t 0 for success
 */
int32_t sgi_irq(void *arg);

/**
 * @brief Enable SGI interrupt
 *
 * @param [in] vec SGI number
 */
void enable_sgi_irq(int vec, int32_t (*)(void *arg));
#endif

/**
 * @}
 * @}
 * @}
 *
 */

