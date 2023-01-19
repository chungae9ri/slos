/*
  kernel/arm/smp-vector.S kernel exception handler
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

/* arm exception code */

.extern secondary_start_kernel 
.section SECONDARY_EXCEPTIONS, "ax"

.arm
.set SECONDARY_EXCEPTION_START, 0xC0201000			
.global secondary_exceptions
secondary_exceptions:
	b sec_reset_handler		
	b sec_undefined_instruction_handler
	b sec_syscall_handler
	b sec_prefetch_abort_handler
	b sec_data_abort_handler
	b sec_reserved
	b sec_irq_handler
	b sec_fiq_handler

sec_reset_handler:
	/* read scr */
	mrc 	p15, 0, r0, c1, c1, 0
	/*set VBAR (banked) for secondary core */
	ldr	r0, =SECONDARY_EXCEPTION_START
	mcr	p15,0,r0,c12,c0,0 

	/*change to supervisor*/
	msr	CPSR_c, #MODE_SVC | I_BIT | F_BIT
	
	/*; setup svc stack*/
	ldr	r0,=SEC_SVC_STACK_BASE
	mov	r13, r0

	/*; Switch to undefined mode and setup the undefined mode stack*/
	msr     CPSR_c, #MODE_UND | I_BIT | F_BIT
	ldr 	r0,=SEC_UNDEF_STACK_BASE
	mov     r13, r0

	/*; Switch to abort mode and setup the abort mode stack*/
	msr     CPSR_c, #MODE_ABT | I_BIT | F_BIT
	ldr	r0,=SEC_ABT_STACK_BASE
	mov     r13, r0

	/*; Switch to SYS mode and setup the SYS mode stack*/
	msr     CPSR_c, #MODE_SYS | I_BIT | F_BIT
	ldr	r0,=SEC_SYS_STACK_BASE
	mov     r13, r0

	/*; Switch to IRQ mode and setup the IRQ mode stack*/
	msr     CPSR_c, #MODE_IRQ | I_BIT | F_BIT
	ldr	r0,=SEC_IRQ_STACK_BASE
	mov     r13, r0

	/*; Switch to FIQ mode and setup the FIQ mode stack*/
	msr     CPSR_c, #MODE_FIQ | I_BIT | F_BIT
	ldr	r0,=SEC_FIQ_STACK_BASE
	mov     r13, r0

	/* Return to supervisor mode*/
	msr     CPSR_c, #MODE_SVC
	b	secondary_start_kernel
;
sec_undefined_instruction_handler:
	b	.
;
sec_syscall_handler:
	b 	.
;
sec_prefetch_abort_handler:
	b	.
;
sec_data_abort_handler:
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
sec_reserved:
	b	.
;
sec_irq_handler:
	ldr 	r12, =SEC_CONTEXT_MEM
	sub	r12, #4
	stmfa	r12!, {r0-r11}
	sub 	r0, lr, #4
	msr	cpsr_c, #MODE_SVC | I_BIT | F_BIT  /* irq,fiq disabled */
	mov	r12, #0x422C
	stmfa	r12!, {sp, lr}
	/* r0 should be saved after storing sp, lr 
	 * stmfa r12!, {sp, lr, r0} doesn't work.
	 * Compiler changes the order.
	 */
	str	r0, [r12, #4]
	bl	gic_irq_handler
	;
	mrs	r0, CPSR
	bic	r1, r0, #I_BIT|F_BIT
	msr	cpsr_c, r1
	ldr	r12, =SEC_CONTEXT_MEM_END
	ldmfa	r12!, {r0-r11, sp, lr, pc}
;
sec_fiq_handler:
	b 	.
;
.end
