/*
  kernel/core/ops.S kernel misc operations
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

/*.set CONTEXT_MEM, 0x4000*/

.set LOCKED,0x1
.set UNLOCKED,0x0

.arm
.text

.global dsb
dsb:
	mov 	r0, #0
	mcr 	p15, 0, r0, c7, c10, 4
	bx 	lr
;
.global switch_context_yield
switch_context_yield:
	mov	r12, r0
	str	lr, [r12],#4
	str	lr, [r12],#4
	str	r13, [r12],#4
	stmia	r12, {r0-r11}
	mov 	r12, r1
	add	r12, #56
	ldmda	r12, {r0-r11}
	sub	r12, #48
	ldr	r13, [r12],#-4
	ldr	r14, [r12],#-4
	/* enable interrupt */
	mrs	r1,	CPSR
	bic	r1,	r1,#IF_BIT
	msr	CPSR_c, r1	
	ldr	pc, [r12]
	nop

.global do_switch_context
/* do_switch_context(struct task_struct *curr, struct task_struct *next) */
do_switch_context:
	push	{r0-r4}
	mov 	r2,#0x11
	/*mov	r4,#CONTEXT_MEM*/
	ldr	r4,=CONTEXT_MEM
savectxt:
	ldr	r3,[r4],#4
	str	r3,[r0],#4
	subs	r2,r2,#1
	bne	savectxt
	mov 	r2,#0x11
	/*mov	r4,#CONTEXT_MEM*/
	ldr 	r4,=CONTEXT_MEM
restrctxt:
	ldr	r3,[r1],#4
	str	r3,[r4],#4
	subs	r2,r2,#1
	bne	restrctxt
	pop	{r0-r4}
	mov	pc,lr

.global enable_interrupt
enable_interrupt:
	mrs	r0,	CPSR
	bic	r1,	r0,#IF_BIT
	msr	CPSR_c, r1	
	mov	pc, lr		/* return */

.global disable_interrupt
disable_interrupt:
	mrs	r0,	CPSR
	orr	r1, r0,	#IF_BIT
	msr	CPSR_c, r1	
	mov 	pc, lr		/* return */

.global get_svc
get_svc:
	mrs	r12, cpsr
	and	r0, r12, #MODE_SVC
	mov 	pc, lr
	
.global spin_lock_acquire
spin_lock_acquire:
	ldr	r1,=LOCKED
loop1:	ldrex	r2,[r0]
	cmp	r2,r1
	beq	loop1
	/* store r1 to [r0], r2 is result */
	strexne	r2,r1,[r0] 
	cmpne	r2,#1 
	beq	loop1
	/*lock acquired*/
	DMB
	bx	lr
.global spin_lock_release
spin_lock_release:
	ldr	r1,=UNLOCKED
	DMB
	str	r1,[r0]
	bx	lr
.end
