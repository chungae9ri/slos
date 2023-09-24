/*
  kernel/inc/mem_layout.h 
  (C) 2018 Kwangdo Yi <kwangdo.yi@gmail.com>
 
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, see <http://www.gnu.org/licenses/>
*/

#ifdef __ASSEMBLY__
/* memory map */
.set KERNEL_FRAME_BITMAP,			0xC000 
.set CONTEXT_MEM, 				0x00004000 /* cpu0 secure mode context memory address */
.set CONTEXT_MEM_SP,				0x402C
.set CONTEXT_MEM_END, 				0x00004038
.set SEC_CONTEXT_MEM, 				0x00004200 /* cpu1 secure mode context memory address */
.set SEC_CONTEXT_MEM_SP,			0x422C
.set SEC_CONTEXT_MEM_END, 			0x00004238
.set MON_CONTEXT_MEM_S,     0x00005000
.set MON_CONTEXT_MEM_NS,     0x00006000
/* kernel*/
.set SECURE_KERNEL_EXCEPTION_BASE, 0xC0101000			
.set KERNEL_CODE_BASE, 				0x100000
.set KERNEL_END, 				0x300000
/* CPU 0 secure mode stacks */
.set MON_STACK_BASE,        0x315FFC /* 3M + 4KiB*22 */
.set SVC_STACK_BASE_S, 				0x314FFC /* 3M + 4KiB*21, support 16 kernel threads */
.set SYS_STACK_BASE_S, 				0x304FFC /* 3M + 4KiB*5 */
.set IRQ_STACK_BASE_S, 				0x303FFC /* 3M + 4KiB*4 */
.set FIQ_STACK_BASE_S, 				0x302FFC /* 3M + 4KiB*3 */
.set ABT_STACK_BASE_S, 				0x301FFC /* 3M + 4KiB*2 */
.set UNDEF_STACK_BASE, 				0x300FFC /* 3M + 4KiB */
/* CPU 0 non-secure mode stacks */
.set SVC_STACK_BASE_NS, 				0x334FFC /* 3M + 4KiB*21, support 16 kernel threads */
.set SYS_STACK_BASE_NS, 				0x324FFC /* 3M + 4KiB*5 */
.set IRQ_STACK_BASE_NS, 				0x323FFC /* 3M + 4KiB*4 */
.set FIQ_STACK_BASE_NS, 				0x322FFC /* 3M + 4KiB*3 */
.set ABT_STACK_BASE_NS, 				0x321FFC /* 3M + 4KiB*2 */
.set UNDEF_STACK_BASE_NS, 				0x320FFC /* 3M + 4KiB */
/* blank for CPU 0 normal world stack */
/* CPU 1 secure mode stacks */
.set SEC_SVC_STACK_BASE,			0x354FFC
.set SEC_SYS_STACK_BASE, 			0x344FFC
.set SEC_IRQ_STACK_BASE, 			0x343FFC 
.set SEC_FIQ_STACK_BASE, 			0x342FFC
.set SEC_ABT_STACK_BASE, 			0x341FFC
.set SEC_UNDEF_STACK_BASE, 			0x340FFC
/* blank for CPU 1 normal world stack */
.set KERN_PGD_START_BASE, 			0x400000 /* page directory base, size 16KiB aligned = 1 page directory * 4KEntries * 4B size */
.set KERN_PGT_START_BASE, 			0x404000 /* page table base, size 4MiB = (1024 * 1024) entries * 4 */
/* user app */
.set USER_APP_BASE, 				0xF00000 /* user app base */
/* end of memory map */
/* misc definitions */
.set USER_APP_GAP, 				0x100000  /* 1MB user app gap */
.set TASK_STACK_GAP, 				0x1000 /* 4k */
.set MODE_SVC, 					0x13
.set MODE_ABT, 					0x17
.set MODE_MON,          0x16
.set MODE_UND, 					0x1b
.set MODE_SYS, 					0x1f
.set MODE_FIQ, 					0x11
.set MODE_IRQ, 					0x12
.set I_BIT, 					0x80
.set F_BIT, 					0x40
.set IF_BIT, 					0xC0
.set NS_BIT,          0x01
.set CONTEXT_MEM_LEN, 				0xF
#else
/* memory map */
#define KERNEL_FRAME_BITMAP			0xC000 
#define CONTEXT_MEM 				0x00004000 /* cpu0 secure mode context memory address */
#define CONTEXT_MEM_SP				0x402C
#define CONTEXT_MEM_END				0x00004038
#define SEC_CONTEXT_MEM				0x00004200 /* cpu1 secure mode context memory address */
#define SEC_CONTEXT_MEM_SP			0x422C
#define SEC_CONTEXT_MEM_END 			0x00004238
/* kernel*/
#define KERNEL_CODE_BASE			0x100000
#define KERNEL_END 				0x300000
/* CPU 0 secure mode stacks */
#define SVC_STACK_BASE 				0x314FFC /* 3M + 4KiB*21 support 16 kernel threads */
#define SYS_STACK_BASE 				0x304FFC /* 3M + 4KiB*5 */
#define IRQ_STACK_BASE 				0x303FFC /* 3M + 4KiB*4 */
#define FIQ_STACK_BASE 				0x302FFC /* 3M + 4KiB*3 */
#define ABT_STACK_BASE 				0x301FFC /* 3M + 4KiB*2 */
#define UNDEF_STACK_BASE			0x300FFC /* 3M + 4KiB */
/* blank for CPU 0 normal world stack */
/* CPU 1 secure mode stacks */
#define SEC_SVC_STACK_BASE			0x354FFC
#define SEC_SYS_STACK_BASE			0x344FFC
#define SEC_IRQ_STACK_BASE 			0x343FFC 
#define SEC_FIQ_STACK_BASE 			0x342FFC
#define SEC_ABT_STACK_BASE 			0x341FFC
#define SEC_UNDEF_STACK_BASE 			0x340FFC
/* blank for CPU 1 normal world stack */
#define KERN_PGD_START_BASE 			0x400000 /* page directory base, size 16KiB aligned = 1 page directory * 4KEntries * 4B size */
#define KERN_PGT_START_BASE 			0x404000 /* page table base, size 4MiB = (1024 * 1024) entries * 4 */
/* user app */
#define USER_APP_BASE 				0xF00000 /* user app base */
/* peripheral reg address */
/* outstream device regs */
#define ODEV_REG_CTRL				0x43c00000
#define ODEV_REG_STATUS				0x43c00004
#define ODEV_REG_ADDR				0x43c00008
#define ODEV_REG_LEN				0x43c0000c
#define ODEV_REG_LATENCY			0x43c00010
/* modecore device regs */
#define MODCORE_DMA_REG_CNTL			0x43c10000
#define MODCORE_DMA_REG_STATUS			0x43c10004
#define MODCORE_DMA_REG_SRC_ADDR		0x43c10008
#define MODCORE_DMA_REG_LEN			0x43c1000c
#define MODCORE_DMA_REG_DST_ADDR		0x43c10010
/* end of memory map */

/* misc definitions */
#define TASK_STACK_GAP 				0x1000 /* 4k */
#define USER_APP_GAP 				0x100000  /* 1MB user app gap */

#define GB * (0x1 << 30)
#define MB * (0x1 << 20)
#define KB * (0x1 << 10)
#define KERN_FRAME_START ((0 MB) / (4 KB))
#define KERN_FRAME_NUM ((128 MB) / (4 KB))
#define HEAP_FRAME_START ((64 MB) / (4 KB))
#define HEAP_FRAME_NUM ((64 MB) / (4 KB))
#define PREALLOC_FRAME_START ((0 MB) / (4 KB))
#define PREALLOC_FRAME_NUM ((64 MB) / (4 KB))
#endif
