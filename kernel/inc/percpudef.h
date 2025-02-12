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
 * @brief Macro definitions for per cpu operation
 *
 */

#ifndef _PER_CPU_DEF_
#define _PER_CPU_DEF_

#ifdef _ENABLE_SMP_
#include <ops.h>

/** CPU number in the system */
#define NR_CPUS 2

extern unsigned long __per_cpu_offset[NR_CPUS];
/** Per cpu offset */
#define per_cpu_offset(x) (__per_cpu_offset[x])
/** Separate out the type, so (int[3], foo) works. */
#define DEFINE_PER_CPU(type, name) __section(".data.percpu") __typeof__(type) per_cpu_##name
/** var is in discarded region: offset to particular copy we want */
#define RELOC_HIDE(ptr, off)                                                                       \
	({                                                                                         \
		unsigned long __ptr;                                                               \
		__ptr = (unsigned long)ptr;                                                        \
		(typeof(ptr))(__ptr + off);                                                        \
	})

/** Per cpu defintion */
#define per_cpu(var, cpu) (*RELOC_HIDE(&per_cpu_##var, __per_cpu_offset[cpu]))
/** Get variable per cpu */
#define __get_cpu_var(var) per_cpu(var, smp_processor_id())
/** Relocated address */
#define RELOC_ADDR(ptr, off)                                                                       \
	({                                                                                         \
		unsigned long __ptr;                                                               \
		__ptr = (unsigned long)ptr;                                                        \
		(unsigned long)(__ptr + off);                                                      \
	})

/** Per cpu address */
#define per_cpu_addr(var, cpu) (RELOC_ADDR(&per_cpu_##var, __per_cpu_offset[cpu]))
/** Get per cpu variable address */
#define __get_cpu_var_addr(var) per_cpu_addr(var, smp_processor_id())

#endif
#endif

/**
 * @}
 * @}
 * @}
 *
 */
