/**
 * @addtogroup kernel 
 * @{
 * @addtogroup kernel_arch Arch
 * @{
 * @addtogroup kernel_arch_ca9 Cortex-A9
 * @{
 * 
 * @file
 * @brief Exception vector table for primary CPU 
 */

// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

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
	/*set VBAR (banked) with 0x100000 */
	ldr	r0, =KERNEL_EXCEPTION_START
	mcr	p15,0,r0,c12,c0,0 

	 /*change to supervisor*/
	msr	CPSR_c, #MODE_SVC | I_BIT | F_BIT
	
	/* setup svc stack*/
	ldr	r0,=SVC_STACK_BASE
	mov	r13, r0

	/*; Switch to undefined mode and setup the undefined mode stack*/
	msr     CPSR_c, #MODE_UND | I_BIT | F_BIT
	ldr 	r0,=UNDEF_STACK_BASE
	mov     r13, r0

	/* Switch to abort mode and setup the abort mode stack*/
	msr     CPSR_c, #MODE_ABT | I_BIT | F_BIT
	ldr	r0,=ABT_STACK_BASE
	mov     r13, r0

	/* Switch to SYS mode and setup the SYS mode stack*/
	msr     CPSR_c, #MODE_SYS | I_BIT | F_BIT
	ldr	r0,=SYS_STACK_BASE
	mov     r13, r0

	/* Switch to IRQ mode and setup the IRQ mode stack*/
	msr     CPSR_c, #MODE_IRQ | I_BIT | F_BIT
	ldr	r0,=IRQ_STACK_BASE
	mov     r13, r0

	/* Switch to FIQ mode and setup the FIQ mode stack*/
	msr     CPSR_c, #MODE_FIQ | I_BIT | F_BIT
	ldr	r0,=FIQ_STACK_BASE
	mov     r13, r0

	/* enable FPU
	 * step 1: enable FPU access from CPACR (Coprocessor Access Control register)
	 */
	mrc p15, 0, r0, c1, c0, 2  /* Read CPACR into r0 */
	orr r0, r0, #(0xF << 20)   /* Set cp10 and cp11 to full access */
	mcr p15, 0, r0, c1, c0, 2  /* Write back to CPACR */
	isb                        /* Ensure changes take effect */
	/* step 2: enable FPU from FPEXC (FPU exception control register) */
	mrc p10, 7, r0, c8, c0, 3  /* Read FPEXC into r0 */
	orr r0, r0, #0x40000000    /* Set the EN bit (bit 30) */
	mcr p10, 7, r0, c8, c0, 3  /* Write back to FPEXC */

	/* Return to supervisor mode*/
	msr     CPSR_c, #MODE_SVC
	b	start_kernel
;
undefined_instruction_handler:
	b	platform_undefined_handler
;
syscall_handler:
	msr	cpsr_c, #MODE_SVC | I_BIT | F_BIT
	stmfd	sp!, {r0-r12,lr}
	ldr 	r12, [lr,#-4]
	bic	r12, #0xff000000
	/* r0 for message buffer, r1 is idx for user task
	   r2 is for syscall number
	 */
	mov 	r2, r12
	bl	platform_syscall_handler
	mrs	r0, CPSR
	bic	r1, r0, #I_BIT|F_BIT
	msr     cpsr_c, r1
	ldmfd	sp!, {r0-r12,pc}
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
	ldr 	r12, =CONTEXT_MEM
	sub	r12, #4
	stmfa	r12!, {r0-r11}
	sub 	r0, lr, #4
	msr	cpsr_c, #MODE_SVC | I_BIT | F_BIT  /* irq,fiq disabled */
	mov	r12, #CONTEXT_MEM_SP
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
	ldr	r12, =CONTEXT_MEM_END
	ldmfa	r12!, {r0-r11, sp, lr, pc}

fiq_handler:
	b 	.
;
.end

/**
 * @}
 * @}
 * @}
 */