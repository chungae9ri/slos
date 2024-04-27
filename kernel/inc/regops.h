// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

#include <stdint.h>

#if defined(__aarch64__)
typedef uint64_t    arch_addr_t;
#else
typedef uint32_t    arch_addr_t;
#endif

static inline uint8_t read8(arch_addr_t addr)
{
    return (uint8_t)(*(volatile uint8_t *)addr);
}

static inline void write8(arch_addr_t addr, uint8_t val)
{
    *(volatile uint8_t *)addr = val;
}

static inline uint16_t read16(arch_addr_t addr)
{
    return (uint16_t)(*(volatile uint16_t *)addr);
}

static inline void write16(arch_addr_t addr, uint16_t val)
{
    *(volatile uint16_t *)addr = val;
}

static inline uint32_t read32(arch_addr_t addr)
{
    return (uint32_t)(*(volatile uint32_t *)addr);
}

static inline void write32(arch_addr_t addr, uint32_t val)
{
    *(volatile uint32_t *)addr = val;
}

static inline uint64_t read64(arch_addr_t addr)
{
    return (uint64_t)(*(volatile uint64_t *)addr);
}

static inline void write64(arch_addr_t addr, uint64_t val)
{
    *(volatile uint64_t *)addr = val;
}