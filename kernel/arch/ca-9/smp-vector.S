/**
 * @addtogroup kernel 
 * @{
 * @addtogroup kernel_arch Arch
 * @{
 * @addtogroup kernel_arch_ca9 Cortex-A9
 * @{
 * 
 * @file
 * @brief Exception vector table for secondary CPU
 */

// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

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
	stmia	r12!, {r0-r11} 
	sub 	r0, lr, #4 /* return address */
	msr		cpsr_c, #MODE_SVC | I_BIT | F_BIT  /* irq,fiq disabled */
	ldr 	r12, =0x5030
	stmia	r12!, {sp, lr}
	/* r0 (return address) should be saved after storing sp, lr 
	 * stmfa r12!, {sp, lr, r0} doesn't work.
	 * Compiler changes the order.
	 */
	str		r0, [r12],#4 /* save return address */
	add		r12, r12,#4	 /* reserved */
	/* save FPU context */
	vstmia 	r12!, {s0-s31}		/* save all 32 single-precision registers */
	mrc 	p10, 7, r0, c1, c0, 2	/* read FPSCR */
	str 	r0, [r12],#4		/* save FPSCR to memory */
	mrc 	p10, 7, r0, c8, c0, 3	/* read FPEXC */
	str 	r0, [r12]		/* save FPEXC to memory */
	/**/
	bl	gic_irq_handler
	;
	mrs	r0, CPSR
	bic	r1, r0, #I_BIT|F_BIT /* enable interrupt */
	msr	cpsr_c, r1
	/* restore FPU context */
	mov 	r12, #0x5040 /* start of FPU context */
	vldmia 	r12!, {s0-s31}		/* restore all 32 single-precision registers */
	ldr 	r0, [r12],#4		/* load FPSCR from memory */
	mcr 	p10, 7, r0, c1, c0, 2	/* write FPSCR */
	ldr 	r0, [r12]		/* load FPEXC from memory */
	mcr 	p10, 7, r0, c8, c0, 3	/* write FPEXC */	
	/**/
	ldr		r12, =SEC_CONTEXT_MEM
	/*ldr		r12, =0x503C*/
	ldmia	r12!, {r0-r11, sp, lr, pc}
;
sec_fiq_handler:
	b 	.
;
.end

/**
 * @}
 * @}
 * @}
 */