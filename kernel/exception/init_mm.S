/*
  kernel/exception/init_mm.S initialization of memory manager
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
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

.include "mem_layout.h"
.set MASK_MMU, 			0x00000001
.set MASK_DCACHE,		0x00000004
.set MASK_ICACHE,		0x00001000
.set MASK_ACCESSPERM,		0x20000000

.section SSBL
.arm
.global ssbl
.global invalidate_dcache

ssbl:
	/* set SVC mode, PL1 */
	msr     CPSR_c, #MODE_SVC
	/*invalidate scu*/
	ldr	r7, =0xf8f0000c
	ldr	r6, =0xffff
	str	r6, [r7]

	/* Invalidate caches and TLBs */
	mov	r0,#0				/* r0 = 0  */
	mcr	p15, 0, r0, c8, c7, 0		/* invalidate TLBs */
	mcr	p15, 0, r0, c7, c5, 0		/* invalidate icache */
	mcr	p15, 0, r0, c7, c5, 6		/* Invalidate branch predictor array */
	bl	invalidate_dcache		/* invalidate dcache */

	/* Disable MMU, if enabled */
	mrc	p15, 0, r0, c1, c0, 0		/* read CP15 register 1 */
	bic	r0, r0, #0x1			/* clear bit 0 */
	mcr	p15, 0, r0, c1, c0, 0		/* write value back */

	/*; jump to kernel page table init */
	bl	init_pgt	

	/* set scu enable bit in scu */
	ldr	r7, =0xf8f00000
	ldr	r0, [r7]
	orr	r0, r0, #0x1
	str	r0, [r7]

	/* set the domain access register as manager mode */
	ldr	r0, =0xFFFFFFFF
	mcr 	p15, 0, r0, c3, c0, 0
	/* invalidate I tlb */
	ldr	r0, =0x00000000
	mcr	p15, 0, r0, c8, c5, 0
	/* invalidate D tlb */
	ldr	r0, =0x00000000
	mcr	p15, 0, r0, c8, c6, 0
	/* invalidate unified tlb */
	ldr 	r0, =0x00000000
	mcr	p15, 0, r0, c8, c7, 0
	/* set the TTBCR as 0 which means 
	 * enable page table walk with TTBR0
	 */
	ldr	r0, =0x00000000		
	mcr 	p15, 0, r0, c2, c0, 2
#if 0
	/* set TTBR1, TTBR1 is not used*/
	ldr	r0, =KERN_PGD_START_BASE
	mcr	p15, 0, r0, c2, c0, 1
#endif
	/* set TTBR0 Bit[6:0] = 0x22 means
	   Bit[0]: 1'b0: IRGN[1]: 0 IRGN[1:0] =2'b00 means non-cacheable
	   Bit[1]: 1'b1: S: Shareable
	   Bit[2]: 1'b0: IMP
	   Bit[4:3]: 2'b00: RGN, outer Noncacheable
	   Bit[5]: 1'b1: NOS, Inner shareable
	   Bit[6]: 1'b0: IRGN[0]:0
	 */
	/* set TTBR0 as 0x0 */
	ldr	r0, =KERN_PGD_START_BASE
	/* Outer-cacheable, WB */
	/*orr	r0, r0, #0x5B	*/
	mcr	p15, 0, r0, c2, c0, 0
	/* read control (c1) register of cp 15 */
	mrc	p15, 0, r0, c1, c0, 0	
	orr	r0, r0, #MASK_MMU | MASK_DCACHE
	orr	r0, r0, #MASK_ICACHE
	orr	r0, r0, #MASK_ACCESSPERM
	/* enable MMU, D cache, I cache */
	mcr	p15, 0, r0, c1, c0, 0
	/* Write to ACTLR */
	mrc	p15, 0, r0, c1, c0, 1		/* Read ACTLR*/
	orr	r0, r0, #(0x01 << 6)		/* set SMP bit */
	orr	r0, r0, #(0x01 )		/* Cache/TLB maintenance broadcast */
	mcr	p15, 0, r0, c1, c0, 1		/* Write ACTLR*/
	/* jump to reset handler*/
	ldr 	r0, =reset_handler
	mov	pc, r0
	nop

	/*
	 *************************************************************************
	 *
	 * invalidate_dcache - invalidate the entire d-cache by set/way
	 *
	 * Note: for Cortex-A9, there is no cp instruction for invalidating
	 * the whole D-cache. Need to invalidate each line.
	 *
	 *************************************************************************
	 */
invalidate_dcache:
	mrc	p15, 1, r0, c0, c0, 1		/* read CLIDR */
	ands	r3, r0, #0x7000000
	mov	r3, r3, lsr #23			/* cache level value (naturally aligned) */
	beq	finished
	mov	r10, #0				/* start with level 0 */
loop1:
	add	r2, r10, r10, lsr #1		/* work out 3xcachelevel */
	mov	r1, r0, lsr r2			/* bottom 3 bits are the Cache type for this level */
	and	r1, r1, #7			/* get those 3 bits alone */
	cmp	r1, #2
	blt	skip				/* no cache or only instruction cache at this level */
	mcr	p15, 2, r10, c0, c0, 0		/* write the Cache Size selection register */
	isb					/* isb to sync the change to the CacheSizeID reg */
	mrc	p15, 1, r1, c0, c0, 0		/* reads current Cache Size ID register */
	and	r2, r1, #7			/* extract the line length field */
	add	r2, r2, #4			/* add 4 for the line length offset (log2 16 bytes) */
	ldr	r4, =0x3ff
	ands	r4, r4, r1, lsr #3		/* r4 is the max number on the way size (right aligned) */
	clz	r5, r4				/* r5 is the bit position of the way size increment */
	ldr	r7, =0x7fff
	ands	r7, r7, r1, lsr #13		/* r7 is the max number of the index size (right aligned) */
loop2:
	mov	r9, r4				/* r9 working copy of the max way size (right aligned) */
loop3:
	orr	r11, r10, r9, lsl r5		/* factor in the way number and cache number into r11 */
	orr	r11, r11, r7, lsl r2		/* factor in the index number */
	mcr	p15, 0, r11, c7, c6, 2		/* invalidate by set/way */
	subs	r9, r9, #1			/* decrement the way number */
	bge	loop3
	subs	r7, r7, #1			/* decrement the index */
	bge	loop2
skip:
	add	r10, r10, #2			/* increment the cache number */
	cmp	r3, r10
	bgt	loop1

finished:
	mov	r10, #0				/* swith back to cache level 0 */
	mcr	p15, 2, r10, c0, c0, 0		/* select current cache level in cssr */
	dsb
	isb

	bx	lr
.end
