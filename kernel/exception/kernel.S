/*
  kernel/core/kernel.S kernel exception handler
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

/* arm exception code */

.extern platform_undefined_handler
.extern platform_syscall_handler
.extern platform_prefetch_abort_handler
.extern platform_data_abort_handler
.extern gic_irq_handler
.extern platform_fiq_handler
.extern start_kernel 

.section EXCEPTIONS, "ax"
.arm
.global exceptions
.global reset_handler
.set KERNEL_EXCEPTION_START, 0xC0101000			

exceptions:
	b reset_handler		
	b undefined_instruction_handler
	b syscall_handler
	b prefetch_abort_handler
	b data_abort_handler
	b reserved
	b irq_handler
	b fiq_handler

reset_handler:
	 /*set VBAR with 0x100000 */
	ldr	r0, =KERNEL_EXCEPTION_START
	mcr	p15,0,r0,c12,c0,0 

	 /*change to supervisor*/
	msr	CPSR_c, #MODE_SVC | I_BIT | F_BIT
	
	/*; setup svc stack*/
	ldr	r0,=SVC_STACK_BASE
	mov	r13, r0

	/*; Switch to undefined mode and setup the undefined mode stack*/
	msr     CPSR_c, #MODE_UND | I_BIT | F_BIT
	ldr 	r0,=UNDEF_STACK_BASE
	mov     r13, r0

	/*; Switch to abort mode and setup the abort mode stack*/
	msr     CPSR_c, #MODE_ABT | I_BIT | F_BIT
	ldr	r0,=ABT_STACK_BASE
	mov     r13, r0

	/*; Switch to SYS mode and setup the SYS mode stack*/
	msr     CPSR_c, #MODE_SYS | I_BIT | F_BIT
	ldr	r0,=SYS_STACK_BASE
	mov     r13, r0

	/*; Switch to IRQ mode and setup the IRQ mode stack*/
	msr     CPSR_c, #MODE_IRQ | I_BIT | F_BIT
	ldr	r0,=IRQ_STACK_BASE
	mov     r13, r0

	/*; Switch to FIQ mode and setup the FIQ mode stack*/
	msr     CPSR_c, #MODE_FIQ | I_BIT | F_BIT
	ldr	r0,=FIQ_STACK_BASE
	mov     r13, r0

	/*; Return to supervisor mode*/
	msr     CPSR_c, #MODE_SVC
	b	start_kernel
;
undefined_instruction_handler:
	b	platform_undefined_handler
;
syscall_handler:
	bl	platform_syscall_handler
;
prefetch_abort_handler:
	b	.
;
data_abort_handler:
#if 0
	mrc	p15, 0, r0, c5, c0, 0 /* read dfsr, test code */
	mrc	p15, 0, r1, c5, c1, 0 /* read ifsr, test code */
	mrc	p15, 0, r2, c6, c0, 0 /* read fault address, test code */
	mrc 	p15, 0, r3, c2, c0, 0 /* read TTBR0, test code */
	mrc 	p15, 0, r4, c2, c0, 1 /* read TTBR1, test code */
#endif
	sub	r12, r14, #8
	msr 	cpsr_c, #MODE_SVC | I_BIT | F_BIT
	stmfd	sp!, {r0-r11, r12}
	push	{r14}
	mrc	p15, 0, r0, c5, c0, 0 /* read dfsr, test code */
	bl	platform_data_abort_handler
	pop	{r14}
	mrs	r0, CPSR
	bic	r1, r0, #I_BIT|F_BIT
	msr	cpsr_c, r1
	ldmfd	sp!, {r0-r11, pc}
;
reserved:
	b	.
;
irq_handler:
	stmia 	r13, {r4-r6}
	mov	r4, r13
	sub 	r5, lr, #4
	msr	cpsr_c,#MODE_SVC | I_BIT | F_BIT /* irq/fiq disabled, SVC mode */
	mov	r6,#CONTEXT_MEM
	str	r5, [r6],#4 /* save return addr */
	str	lr, [r6],#4  /* save current task lr */
	str	r13, [r6],#4 /* save sp */
	stmia	r6!, {r0-r3} 
	mov	r1, r6
	ldmia	r4, {r4-r6} /* restore r4-r6 */
	stmia	r1!,{r4-r12}
	mrs	r5, spsr
	str	r5, [r1],#4 /* save spsr */
	bl 	gic_irq_handler
;	
	mov 	r12, #CONTEXT_MEM 
	add 	r12,r12,#4
	ldr	r14,[r12],#4
	ldr	r13,[r12],#4
	ldmia	r12!,{r0-r11}
	add 	r12,r12,#4
	ldr	r12,[r12]
	msr	cpsr_c,#MODE_SVC
	mov 	r12,#CONTEXT_MEM 
	ldr	r12,[r12]
	mov	pc,r12
;

fiq_handler:
	b 	.
;
.end
