/* SPDX-License-Identifier: MIT OR Apache-2.0
 *
 * Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>
 */

/**
 * @addtogroup kernel
 * @{
 * @addtogroup kernel_core Core
 * @{
 * @addtogroup kernel_core_misc Miscellaneous kernel modules
 * @{
 *
 * @brief Low level IO operations
 *
 */

#ifndef _REGOPS_H_
#define _REGOPS_H_

#include <stdint.h>
#include <stddef.h>

/**
 * @brief 8bit wide read from addr
 *
 * @param [in] addr Read address
 * @return uint8_t 8bit read data
 */
static inline uint8_t read8(size_t addr)
{
	return (uint8_t)(*(volatile uint8_t *)addr);
}

/**
 * @brief 8bit wide write to addr
 *
 * @param [in] addr Write address
 * @param [in] val 8bit write data
 */
static inline void write8(size_t addr, uint8_t val)
{
	*(volatile uint8_t *)addr = val;
}

/**
 * @brief 16bit wide read from addr
 *
 * @param [in] addr Read address
 * @return uint8_t 16bit read data
 */
static inline uint16_t read16(size_t addr)
{
	return (uint16_t)(*(volatile uint16_t *)addr);
}

/**
 * @brief 16bit wide write to addr
 *
 * @param [in] addr Write address
 * @param [in] val 16bit write data
 */
static inline void write16(size_t addr, uint16_t val)
{
	*(volatile uint16_t *)addr = val;
}

/**
 * @brief 32bit wide read from addr
 *
 * @param [in] addr Read address
 * @return uint8_t 32bit read data
 */
static inline uint32_t read32(size_t addr)
{
	return (uint32_t)(*(volatile uint32_t *)addr);
}

/**
 * @brief 32bit wide write to addr
 *
 * @param [in] addr Write address
 * @param [in] val 32bit write data
 */
static inline void write32(size_t addr, uint32_t val)
{
	*(volatile uint32_t *)addr = val;
}

/**
 * @brief 64bit wide read from addr
 *
 * @param [in] addr Read address
 * @return uint8_t 64bit read data
 */
static inline uint64_t read64(size_t addr)
{
	return (uint64_t)(*(volatile uint64_t *)addr);
}

/**
 * @brief 64bit wide write to addr
 *
 * @param [in] addr Write address
 * @param [in] val 64bit write data
 */
static inline void write64(size_t addr, uint64_t val)
{
	*(volatile uint64_t *)addr = val;
}

#endif

/**
 * @}
 * @}
 * @}
 *
 */
