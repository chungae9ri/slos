/* SPDX-License-Identifier: MIT OR Apache-2.0
 *
 * Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>
 */

#ifndef _MEM_LAYOUT_H_
#define _MEM_LAYOUT_H_

#ifdef __ASSEMBLY__
/* memory map */
.set KERNEL_FRAME_BITMAP,			0xC000
.set CONTEXT_MEM,				0x00004000 /* cpu0 secure mode context memory address */
.set CONTEXT_MEM_SP,				0x402C
.set CONTEXT_MEM_END,				0x00004038
.set SEC_CONTEXT_MEM,				0x00005000 /* cpu1 secure mode context memory address */
.set SEC_CONTEXT_MEM_SP,			0x502C
.set SEC_CONTEXT_MEM_END,			0x00005038
/* kernel*/
.set KERNEL_CODE_BASE,				0x100000
.set KERNEL_END,				0x300000
/* CPU 0 secure mode stacks */
.set SVC_STACK_BASE,				0x314FFC /* 3M + 4KiB*21 support 16 kernel threads */
.set SYS_STACK_BASE,				0x304FFC /* 3M + 4KiB*5 */
.set IRQ_STACK_BASE,				0x303FFC /* 3M + 4KiB*4 */
.set FIQ_STACK_BASE,				0x302FFC /* 3M + 4KiB*3 */
.set ABT_STACK_BASE,				0x301FFC /* 3M + 4KiB*2 */
.set UNDEF_STACK_BASE,				0x300FFC /* 3M + 4KiB */
/* blank for CPU 0 normal world stack */
/* CPU 1 secure mode stacks */
.set SEC_SVC_STACK_BASE,			0x354FFC
.set SEC_SYS_STACK_BASE,			0x344FFC
.set SEC_IRQ_STACK_BASE,			0x343FFC
.set SEC_FIQ_STACK_BASE,			0x342FFC
.set SEC_ABT_STACK_BASE,			0x341FFC
.set SEC_UNDEF_STACK_BASE,			0x340FFC
/* blank for CPU 1 normal world stack */
.set KERN_PGD_START_BASE,			0x400000 /* page directory base, size 16KiB aligned = 1 page directory * 4KEntries * 4B size */
.set KERN_PGT_START_BASE,			0x404000 /* page table base, size 4MiB = (1024 * 1024) entries * 4 */
/* user app */
.set USER_APP_BASE,				0x1000000 /* user app base */
/* ramdisk */
.set RAMDISK_FS_BASE,				0x3000000 /* 48M */
/* end of memory map */
/* misc definitions */
.set USER_APP_GAP,				0x100000  /* 1MB user app gap */
.set TASK_STACK_GAP,				0x1000 /* 4k */
.set MODE_SVC,					0x13
.set MODE_ABT,					0x17
.set MODE_UND,					0x1b
.set MODE_SYS,					0x1f
.set MODE_FIQ,					0x11
.set MODE_IRQ,					0x12
.set I_BIT,					0x80
.set F_BIT,					0x40
.set IF_BIT,					0xC0
.set CONTEXT_MEM_LEN,				0xF
#else
/* memory map */
#define KERNEL_FRAME_BITMAP			0xC000
#define CONTEXT_MEM				0x00004000 /* cpu0 secure mode context memory address */
#define CONTEXT_MEM_SP				0x402C
#define CONTEXT_MEM_END				0x00004038
#define SEC_CONTEXT_MEM				0x00005000 /* cpu1 secure mode context memory address */
#define SEC_CONTEXT_MEM_SP			0x502C
#define SEC_CONTEXT_MEM_END			0x00005038
/* kernel*/
#define KERNEL_CODE_BASE			0x100000
#define KERNEL_END				0x300000
/* CPU 0 secure mode stacks */
#define SVC_STACK_BASE				0x314FFC /* 3M + 4KiB*21 support 16 kernel threads */
#define SYS_STACK_BASE				0x304FFC /* 3M + 4KiB*5 */
#define IRQ_STACK_BASE				0x303FFC /* 3M + 4KiB*4 */
#define FIQ_STACK_BASE				0x302FFC /* 3M + 4KiB*3 */
#define ABT_STACK_BASE				0x301FFC /* 3M + 4KiB*2 */
#define UNDEF_STACK_BASE			0x300FFC /* 3M + 4KiB */
/* blank for CPU 0 normal world stack */
/* CPU 1 secure mode stacks */
#define SEC_SVC_STACK_BASE			0x354FFC
#define SEC_SYS_STACK_BASE			0x344FFC
#define SEC_IRQ_STACK_BASE			0x343FFC
#define SEC_FIQ_STACK_BASE			0x342FFC
#define SEC_ABT_STACK_BASE			0x341FFC
#define SEC_UNDEF_STACK_BASE			0x340FFC
/* blank for CPU 1 normal world stack */
#define KERN_PGD_START_BASE			0x400000 /* page directory base, size 16KiB aligned = 1 page directory 4KEntries * 4B size */
#define KERN_PGT_START_BASE			0x404000 /* page table base, size 4MiB = (1024 * 1024) entries * 4 */
/* user app */
#define USER_APP_BASE				0x1000000  /* user app base */
/* ramdisk */
#define SCRATCH_BASE				0x02000000 /* 32M */
#define RAMDISK_FS_BASE				0x3000000  /* 48M */
/* peripheral reg address */
/* end of memory map */

/* misc definitions */
#define TASK_STACK_GAP				0x1000     /* 4k */
#define USER_APP_GAP				0x100000   /* 1MB user app gap */

#define KERN_FRAME_START			(0x00000000U)
#define KERN_FRAME_NUM				((128 * (0x1 << 20)) / (4 * (0x1 << 10)))
#define HEAP_FRAME_START			((64 * (0x1 << 20)) / (4 * (0x1 << 10)))
#define HEAP_FRAME_NUM				((64 * (0x1 << 20)) / (4 * (0x1 << 10)))
#define PREALLOC_FRAME_START			(0x00000000U)
#define PREALLOC_FRAME_NUM			((64 * (0x1 << 20)) / (4 * (0x1 << 10)))
#endif

#endif

