/* arm exception code */

#include "mem_layout.h"

/* CPSR mode and bit definitions*/
.set MODE_SVC, 0x13
.set MODE_ABT, 0x17
.set MODE_UND, 0x1b
.set MODE_SYS, 0x1f
.set MODE_FIQ, 0x11
.set MODE_IRQ, 0x12
.set I_BIT, 0x80
.set F_BIT, 0x40
.set IF_BIT, 0xC0
.set CONTEXT_MEM, 0x4000
.set SP_MEM, 0x4100

.extern platform_undefined_handler
.extern platform_syscall_handler
.extern platform_prefetch_abort_handler
.extern platform_data_abort_handler
/*.extern platform_irq*/
/*.extern platform_fiq*/
.extern main 
/*.extern USER_CODE_BASE*/

.section EXCEPTIONS, "ax"
.arm
.global exceptions
exceptions :
	/*b reset_handler		*/
	b _boot
	b undefined_instruction_handler
	b syscall_handler	
	b prefetch_abort_handler
	b data_abort_handler
	b reserved	
	b irq_handler
	b fiq_handler

@ Exception handler
reset_handler:
	 /*set VBAR with 0x8000*/
	ldr	r0, =0x8000
	mcr	p15,0,r0,c12,c0,0 

	 /*change to supervisor*/
	@msr	CPSR_c, #MODE_SVC:OR:I_Bit:OR:F_Bit 
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

	/*; jump to main */
	b	main	
@
undefined_instruction_handler:
	stmfd	sp!, {r0-r12, r14}
	sub	sp, sp,#12
	mov	r0, sp
	mrs	r1, spsr
	stmia	r0, {r1, r13-r14}^
	b	platform_undefined_handler
	b	.
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
	ldmfd	sp!, {r0-r12,pc}^
;
prefetch_abort_handler:
	stmfd	sp!, {r0-r12, r14}
	sub	sp, sp,#12
	mov 	r0, sp
	mrs	r1, spsr
	stmia	r0, {r1, r13-r14}^
	b	platform_prefetch_abort_handler
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
	bl	platform_abort_handler
	pop	{r14}
	mrs	r0, CPSR
	bic	r1, r0, #I_BIT|F_BIT
	/*msr	cpsr_c, r1*/
	ldmfd	sp!, {r0-r11, pc}^	
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
#ifdef USE_MMU
	mrc	p15, 0, r12, c2, c0, 0 /* read ttb addr */
	str	r12, [r1] /* str ttb addr to Mem */
#endif
	/*kyi imsi for test bl 	platform_irq*/
;	
	mov 	r12, #CONTEXT_MEM 
	add 	r12,r12,#4
	ldr	r14,[r12],#4
	ldr	r13,[r12],#4
	ldmia	r12!,{r0-r11}
	add 	r12,r12,#4
	ldr	r12,[r12]
	msr 	cpsr_c, r12
	/*msr 	spsr_cxsf, r12*/
#ifdef USE_MMU
	mov 	r12, #CONTEXT_MEM
	add 	r12, #0x44
	ldr 	r12, [r12] /* load ttb addr */
	mcr	p15, 0, r12, c2, c0, 0 /* write ttb addr */
#endif
	mov 	r12,#CONTEXT_MEM 
	ldr	r12,[r12]
	movs	pc,r12
;

fiq_handler:
	sub	lr, lr,#4
	stmfd	sp!, {r0-r3, r12, lr}
	/*kyi imsi for test bl 	platform_fiq*/
	ldmfd	sp!, {r0-r3, r12, pc}^
;
@ end of program
@END
.end
