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
.set KERNEL_CODE_BASE, 				0x100000
.set USER_APP_BASE, 				0x1000000 /* user app base */
.set USER_APP_GAP, 				0x100000  /* 1MB user app gap */
.set KERNEL_END, 				0x300000
/* CPU0 monitor mode stacks */
.set MON_FIQ_STACK_BASE,			0x380FFC
.set MON_IRQ_STACK_BASE,			0x370FFC
.set MON_PRE_ABT_STACK_BASE,			0x360FFC
.set MON_ABT_STACK_BASE,			0x350FFC
.set MON_SMC_STACK_BASE,			0x340FFC
/* CPU0 secure mode stacks */
.set SECURE_SVC_STACK_BASE, 			0x334FFC
.set SECURE_SYS_STACK_BASE, 			0x324FFC 
.set SECURE_IRQ_STACK_BASE, 			0x323FFC
.set SECURE_FIQ_STACK_BASE, 			0x322FFC
.set SECURE_ABT_STACK_BASE, 			0x321FFC
.set SECURE_UNDEF_STACK_BASE,			0x320FFC
/* CPU 0 non-secure mode stacks */
.set SVC_STACK_BASE, 				0x314FFC /* 3M + 4KB*16 support 16 kernel threads */
.set SYS_STACK_BASE, 				0x304FFC /* 3M + 4KB*5 */
.set IRQ_STACK_BASE, 				0x303FFC /* 3M + 4KB*4 */
.set FIQ_STACK_BASE, 				0x302FFC /* 3M + 4KB*3 */
.set ABT_STACK_BASE, 				0x301FFC /* 3M + 4KB*2 */
.set UNDEF_STACK_BASE, 				0x300FFC /* 3M + 4KB */
/* CPU 1 monitor mode stacks */
.set SEC_MON_FIQ_STACK_BASE,			0x480FFC
.set SEC_MON_IRQ_STACK_BASE,			0x470FFC
.set SEC_MON_PRE_ABT_STACK_BASE,		0x460FFC
.set SEC_MON_ABT_STACK_BASE,			0x450FFC
.set SEC_MON_SMC_STACK_BASE,			0x440FFC
/* CPU 1 secure mode stacks */
.set SEC_SECURE_SVC_STACK_BASE, 		0x434FFC
.set SEC_SECURE_SYS_STACK_BASE,			0x424FFC 
.set SEC_SECURE_IRQ_STACK_BASE, 		0x423FFC
.set SEC_SECURE_FIQ_STACK_BASE,			0x422FFC
.set SEC_SECURE_ABT_STACK_BASE, 		0x421FFC
.set SEC_SECURE_UNDEF_STACK_BASE,		0x420FFC
/* CPU 1 non-secure mode stacks */
.set SEC_SVC_STACK_BASE,			0x414FFC /* 4M + 4KB*16 support 16 kernel threads */
.set SEC_SYS_STACK_BASE, 			0x404FFC /* 4M + 4KB*5 */
.set SEC_IRQ_STACK_BASE, 			0x403FFC /* 4M + 4KB*4 */
.set SEC_FIQ_STACK_BASE, 			0x402FFC /* 4M + 4KB*3 */
.set SEC_ABT_STACK_BASE, 			0x401FFC /* 4M + 4KB*2 */
.set SEC_UNDEF_STACK_BASE, 			0x400FFC /* 4M + 4KB */
/**/
.set TASK_STACK_GAP, 				0x1000 /* 4k */
.set RAMDISK_FS_BASE, 				0x3000000 /* 48M */
.set KERN_PGT_START_BASE, 			0x41A000 /* page table base 4MB = 4K page tables * 256 entries * 4B size */
.set KERN_PGD_START_BASE, 			0x00008000 /* page directory base at 16KB, size 16KB aligned = 1 page directory * 4KEntries * 4B size */
/*.set KERNEL_FRAME_BITMAP,			0xC000 */ /* need 1 bitmap for 4MB kernel memory */
.set KERNEL_FRAME_BITMAP,			0x415000 /* need 1 bitmap for 4MB kernel memory */
.set KENEL_END,					0x800000
.set USERTASK_START,				0x800000
.set MODE_SVC, 					0x13
.set MODE_ABT, 					0x17
.set MODE_UND, 					0x1b
.set MODE_SYS, 					0x1f
.set MODE_FIQ, 					0x11
.set MODE_IRQ, 					0x12
.set MODE_MON, 					0x16
.set I_BIT, 					0x80
.set F_BIT, 					0x40
.set IF_BIT, 					0xC0
.set CONTEXT_MEM, 				0x00004000 /* cpu0 normal mode context memory address */
.set SECURE_CONTEXT_MEM,			0x00004100 /* cpu0 secure mode context memory address*/	
.set SEC_CONTEXT_MEM, 				0x00004200 /* cpu1 normal mode context memory address */
.set SEC_SECURE_CONTEXT_MEM, 			0x00004300 /* cpu1 secure mode context memory address */
.set CONTEXT_MEM_END, 				0x00004038
.set SEC_CONTEXT_MEM_END, 			0x00004238
.set CONTEXT_MEM_LEN, 				0xF
.set MON_VBAR,					0x102000
.set SEC_MON_VBAR,				0x202000
/*.set SP_MEM, 0x4100*/
#else
#define ODEV_REG_CTRL				0x43c00000
#define ODEV_REG_STATUS				0x43c00004
#define ODEV_REG_ADDR				0x43c00008
#define ODEV_REG_LEN				0x43c0000c
#define ODEV_REG_LATENCY			0x43c00010
#define MODCORE_DMA_REG_CNTL			0x43c10000
#define MODCORE_DMA_REG_STATUS			0x43c10004
#define MODCORE_DMA_REG_SRC_ADDR		0x43c10008
#define MODCORE_DMA_REG_LEN			0x43c1000c
#define MODCORE_DMA_REG_DST_ADDR		0x43c10010
#define KERNEL_CODE_BASE			0x100000
#define USER_APP_BASE 				0x1000000 /* user app base */
#define USER_APP_GAP 				0x100000  /* 1MB user app gap */
#define KERNEL_END 				0x300000
/* CPU0 monitor mode stacks */
#define MON_FIQ_STACK_BASE			0x380FFC
#define MON_IRQ_STACK_BASE			0x370FFC
#define MON_PRE_ABT_STACK_BASE			0x360FFC
#define MON_ABT_STACK_BASE			0x350FFC
#define MON_SMC_STACK_BASE			0x340FFC
/* CPU0 secure mode stacks */
#define SECURE_SVC_STACK_BASE 			0x334FFC
#define SECURE_SYS_STACK_BASE 			0x324FFC 
#define SECURE_IRQ_STACK_BASE 			0x323FFC
#define SECURE_FIQ_STACK_BASE 			0x322FFC
#define SECURE_ABT_STACK_BASE 			0x321FFC
#define SECURE_UNDEF_STACK_BASE			0x320FFC
/* CPU0 non-secure mode stacks */
#define SVC_STACK_BASE 				0x314FFC /* 3M + 4KB*16 support 16 kernel threads */
#define SYS_STACK_BASE 				0x304FFC /* 3M + 4KB*5 */
#define IRQ_STACK_BASE 				0x303FFC /* 3M + 4KB*4 */
#define FIQ_STACK_BASE 				0x302FFC /* 3M + 4KB*3 */
#define ABT_STACK_BASE 				0x301FFC /* 3M + 4KB*2 */
#define UNDEF_STACK_BASE			0x300FFC /* 3M + 4KB */
/* CPU 1 monitor mode stacks */
#define SEC_MON_FIQ_STACK_BASE			0x480FFC
#define SEC_MON_IRQ_STACK_BASE			0x470FFC
#define SEC_MON_PRE_ABT_STACK_BASE		0x460FFC
#define SEC_MON_ABT_STACK_BASE			0x450FFC
#define SEC_MON_SMC_STACK_BASE			0x440FFC
/* CPU 1 secure mode stacks */
#define SEC_SECURE_SVC_STACK_BASE 		0x434FFC
#define SEC_SECURE_SYS_STACK_BASE 		0x424FFC 
#define SEC_SECURE_IRQ_STACK_BASE 		0x423FFC
#define SEC_SECURE_FIQ_STACK_BASE		0x422FFC
#define SEC_SECURE_ABT_STACK_BASE 		0x421FFC
#define SEC_SECURE_UNDEF_STACK_BASE		0x420FFC
/* CPU 1 non-secure mode stacks */
#define SEC_SVC_STACK_BASE 			0x414FFC /* 4M + 4KB*16 support 16 kernel threads */
#define SEC_SYS_STACK_BASE 			0x404FFC /* 4M + 4KB*5 */
#define SEC_IRQ_STACK_BASE 			0x403FFC /* 4M + 4KB*4 */
#define SEC_FIQ_STACK_BASE 			0x402FFC /* 4M + 4KB*3 */
#define SEC_ABT_STACK_BASE 			0x401FFC /* 4M + 4KB*2 */
#define SEC_UNDEF_STACK_BASE			0x400FFC /* 4M + 4KB */
/**/
#define TASK_STACK_GAP 				0x1000 /* 4k */
#define RAMDISK_FS_BASE				0x03000000 /* 48M */
#define SCRATCH_BASE				0x02000000 /* 32M */
#define CONTEXT_MEM 				0x00004000 /* cpu0 normal mode context memory address */
#define SECURE_CONTEXT_MEM			0x00004100 /* cpu0 secure mode context memory address*/	
#define SEC_CONTEXT_MEM 			0x00004200 /* cpu1 normal mode context memory address */
#define SEC_SECURE_CONTEXT_MEM 			0x00004300 /* cpu1 secure mode context memory address */
#define MON_VBAR				0x102000
#define SEC_MON_VBAR				0x202000
#define KERN_PGT_START_BASE 			0x41A000 /* page table base 4MB = 4K page tables * 256 entries * 4B size */
#define KERN_PGD_START_BASE			0x00008000 /* page directory base16KB = 1 page directory * 4KEntries * 4B size */
/*#define KERNEL_FRAME_BITMAP			0xC000 */ /* need 1 bitmap for 4MB kernel memory */
#define KERNEL_FRAME_BITMAP			0x315000 /* need 1 bitmap for 4MB kernel memory */

#define USERTASK_START				0x800000
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
