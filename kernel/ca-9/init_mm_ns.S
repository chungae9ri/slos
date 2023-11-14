/*
  kernel/arm/init_mm.S initialization of memory manager
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

.include "mem_layout.h"
.set MASK_MMU, 			0x00000001
.set MASK_DCACHE,		0x00000004
.set MASK_ICACHE,		0x00001000
.set MASK_AFE,			0x20000000

.section SSBL_NS
.arm
.global ssbl_ns

.extern reset_handler_ns
.extern init_pgt_ns

ssbl_ns:
	/* set SVC mode, PL1 */
	msr     CPSR_c, #MODE_SVC
	/* read SCR for test */
	mrc p15, 0, r0, c1, c1, 0 
	/* jump to translation table init 
	 * if current cpu is 0
	 */
	bleq	init_pgt_ns			/* cpu0 sets up pgd, pgt */
	
	/* set the TTBCR as 0 which means 
	 * enable page table walk with TTBR0
	 */
	ldr	r0, =0x00000000		
	mcr 	p15, 0, r0, c2, c0, 2
#if 0
	/* set TTBR1, TTBR1 is not used*/
	ldr	r0, =KERN_PGD_START_BASE_NS
	mcr	p15, 0, r0, c2, c0, 1
#endif
	/* set TTBR0 Bit[6:0] = 0x22 means
	 * Bit[0]: 1'b0: IRGN[1]: 0 IRGN[1:0] =2'b00 means non-cacheable
	 * Bit[1]: 1'b1: S: Shareable
	 * Bit[2]: 1'b0: IMP
	 * Bit[4:3]: 2'b00: RGN, outer Noncacheable
	 * Bit[5]: 1'b1: NOS, Inner shareable
	 * Bit[6]: 1'b0: IRGN[0]:0
	 */
	ldr	r0, =KERN_PGD_START_BASE_NS
	mcr	p15, 0, r0, c2, c0, 0
	ldr 	r0, =reset_handler_ns
	mov	pc, r0
	nop

.end
