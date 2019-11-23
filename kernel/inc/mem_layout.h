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
.set KERNEL_END, 				0x200000
.set SVC_STACK_BASE, 				0x214FFC /* 3M + 4KB*16 support 16 kernel threads */
.set SYS_STACK_BASE, 				0x204FFC /* 3M + 4KB*5 */
.set IRQ_STACK_BASE, 				0x203FFC /* 3M + 4KB*4 */
.set FIQ_STACK_BASE, 				0x202FFC /* 3M + 4KB*3 */
.set ABT_STACK_BASE, 				0x201FFC /* 3M + 4KB*2 */
.set UNDEF_STACK_BASE, 				0x200FFC /* 3M + 4KB */
.set TASK_STACK_GAP, 				0x1000 /* 4k */
.set RAMDISK_FS_BASE, 				0x3000000 /* 48M */
.set CONTEXT_MEM, 				0x200000
.set KERN_PGT_START_BASE, 			0x21A000 /* page table base 4MB = 4K page tables * 256 entries * 4B size */
.set KERN_PGD_START_BASE, 			0x00000000 /* page directory base16KB = 1 page directory * 4KEntries * 4B size */
.set KERNEL_FRAME_BITMAP,			0x215000 /* need 1 bitmap for 4MB kernel memory */
.set KENEL_END,					0x800000
.set USERTASK_START,				0x800000
.set MODE_SVC, 					0x13
.set MODE_ABT, 					0x17
.set MODE_UND, 					0x1b
.set MODE_SYS, 					0x1f
.set MODE_FIQ, 					0x11
.set MODE_IRQ, 					0x12
.set I_BIT, 					0x80
.set F_BIT, 					0x40
.set IF_BIT, 					0xC0
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
//
#define KERNEL_CODE_BASE			0x100000
#define USER_APP_BASE 				0x1000000 /* user app base */
#define USER_APP_GAP 				0x100000  /* 1MB user app gap */
#define KERNEL_END 				0x200000
#define SVC_STACK_BASE 				0x214FFC /* 3M + 4KB*16 support 16 kernel threads */
#define SYS_STACK_BASE 				0x204FFC /* 3M + 4KB*5 */
#define IRQ_STACK_BASE 				0x203FFC /* 3M + 4KB*4 */
#define FIQ_STACK_BASE 				0x202FFC /* 3M + 4KB*3 */
#define ABT_STACK_BASE 				0x201FFC /* 3M + 4KB*2 */
#define UNDEF_STACK_BASE			0x200FFC /* 3M + 4KB */
#define TASK_STACK_GAP 				0x1000 /* 4k */
#define RAMDISK_FS_BASE				0x3000000 /* 48M */
#define SCRATCH_BASE				0x2000000 /* 32M */
#define CONTEXT_MEM 				0x200000
#define KERN_PGT_START_BASE 			0x21A000 /* page table base 4MB = 4K page tables * 256 entries * 4B size */
#define KERN_PGD_START_BASE			0x00000000 /* page directory base16KB = 1 page directory * 4KEntries * 4B size */
//#define KERN_PGD_START_BASE			0x216000 /* page directory base16KB = 1 page directory * 4KEntries * 4B size */
#define KERNEL_FRAME_BITMAP			0x215000 /* need 1 bitmap for 4MB kernel memory */

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
