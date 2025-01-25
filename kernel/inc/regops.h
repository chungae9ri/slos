/* SPDX-License-Identifier: MIT OR Apache-2.0
 *
 * Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>
 */

#ifndef _REGOPS_H_
#define _REGOPS_H_

#include <stdint.h>
#include <stddef.h>

static inline uint8_t read8(size_t addr)
{
	return (uint8_t)(*(volatile uint8_t *)addr);
}

static inline void write8(size_t addr, uint8_t val)
{
	*(volatile uint8_t *)addr = val;
}

static inline uint16_t read16(size_t addr)
{
	return (uint16_t)(*(volatile uint16_t *)addr);
}

static inline void write16(size_t addr, uint16_t val)
{
	*(volatile uint16_t *)addr = val;
}

static inline uint32_t read32(size_t addr)
{
	return (uint32_t)(*(volatile uint32_t *)addr);
}

static inline void write32(size_t addr, uint32_t val)
{
	*(volatile uint32_t *)addr = val;
}

static inline uint64_t read64(size_t addr)
{
	return (uint64_t)(*(volatile uint64_t *)addr);
}

static inline void write64(size_t addr, uint64_t val)
{
	*(volatile uint64_t *)addr = val;
}

#endif
