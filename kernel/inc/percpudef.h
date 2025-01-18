// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

#ifndef _PER_CPU_DEF_
#define _PER_CPU_DEF_

#ifdef _ENABLE_SMP_

extern uint32_t smp_processor_id(void);

#define NR_CPUS 2
extern unsigned long __per_cpu_offset[NR_CPUS];

#define per_cpu_offset(x) (__per_cpu_offset[x])

/* Separate out the type, so (int[3], foo) works. */
#define DEFINE_PER_CPU(type, name)                                                                 \
	__attribute__((__section__(".data.percpu"))) __typeof__(type) per_cpu_##name

/* var is in discarded region: offset to particular copy we want */
#define RELOC_HIDE(ptr, off)                                                                       \
	({                                                                                         \
		unsigned long __ptr;                                                               \
		__ptr = (unsigned long)ptr;                                                        \
		(typeof(ptr))(__ptr + off);                                                        \
	})

#define per_cpu(var, cpu)  (*RELOC_HIDE(&per_cpu_##var, __per_cpu_offset[cpu]))
#define __get_cpu_var(var) per_cpu(var, smp_processor_id())
//
#define RELOC_ADDR(ptr, off)                                                                       \
	({                                                                                         \
		unsigned long __ptr;                                                               \
		__ptr = (unsigned long)ptr;                                                        \
		(unsigned long)(__ptr + off);                                                      \
	})

#define per_cpu_addr(var, cpu)  (RELOC_ADDR(&per_cpu_##var, __per_cpu_offset[cpu]))
#define __get_cpu_var_addr(var) per_cpu_addr(var, smp_processor_id())

#endif
#endif
