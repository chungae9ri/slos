/**
 * @addtogroup kernel 
 * @{
 * @addtogroup kernel_arch Arch
 * @{
 * @addtogroup kernel_arch_ca9 Cortex-A9
 * @{
 * 
 * @file
 * @brief Basic Cortex-A9 operations
 */

// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

.include "mem_layout.h"

.set CPU_CONTEXT_SZ,0x40
.set LOCKED,0x1
.set UNLOCKED,0x0

.arm
.text

.global dsb
dsb:
	mov 	r0, #0
	mcr 	p15, 0, r0, c7, c10, 4
	mov	pc, lr
;
.global switch_context_yieldyi
switch_context_yieldyi:
	mov	r12, r0
	/* keep current frame ptr (r11).
	 * r0-r3 will be auto-updated
	 * after returning to the caller (lr).
	 * Don't upte the stack ptr, frame ptr (r11, r13),
	 * and save the previous frame and stack ptr.
	 */
	stmia	r12!, {r0-r11}
	str	sp, [r12],#4
	str	lr, [r12],#4
	/* save current lr to the saved pc */
	str	lr, [r12],#4
	/* save current task's FPU context */
	add 	r12, r12, #4			/* skip reserved */
	vstmia 	r12!, {s0-s31}			/* save all 32 single-precision registers */
	mrc 	p10, 7, r0, c1, c0, 2  		/* read FPSCR */
	str 	r0, [r12],#4			/* save FPSCR to memory */
	mrc 	p10, 7, r0, c8, c0, 3  		/* read FPEXC */
	str 	r0, [r12]			/* save FPEXC to memory */
	/**/
	/* restore FPU context from next task's FPU context */
	add 	r12, r1,#CPU_CONTEXT_SZ		/* set r12 to the start of FPU context */
	vldmia 	r12!, {s0-s31}			/* restore all 31 single-precision registers */
	ldr 	r0, [r12],#4			/* load FPSCR from memory */
	mcr 	p10, 7, r0, c1, c0, 2		/* write FPSCR */
	ldr 	r0, [r12]			/* load FPEXC from memory */
	mcr 	p10, 7, r0, c8, c0, 3		/* write FPEXC */
	/* update regs with new task's ctx */
	mov 	r12, r1
	ldmia	r12!, {r0-r11}
	ldr	sp, [r12],#4
	ldr	lr, [r12],#4
	/* enable interrupt */
	mrs	r1,	CPSR
	bic	r1,	r1,#IF_BIT
	msr	CPSR_c, r1	
	/* jump to the saved pc */
	ldr	pc, [r12]
	nop

.global do_switch_context
/* do_switch_context(struct task_struct *curr, struct task_struct *next) */
do_switch_context:
	push	{r0-r5, lr}
	mov 	r5, r0
	bl 	smp_processor_id
	cmp	r0, #0x1
	beq	secondary_context
	mov 	r2,#CONTEXT_MEM_LEN
	ldr 	r4,=CONTEXT_MEM
	mov 	r0, r5
	mov	r12, r4
	b	savectxt
secondary_context:
	mov 	r0, r5
	mov	r2, #CONTEXT_MEM_LEN
	ldr	r4,=SEC_CONTEXT_MEM
	mov	r12, r4		/* back up context mem addr to r12 */
/* save preempted task's context from CONTEXT_MEM 
 * to current task's context memory ([r0]) 
 */
savectxt:
	ldr	r3,[r4],#4
	str	r3,[r0],#4
	subs 	r2,r2,#1
	bne	savectxt
	/* save FPU context to current task's FPU context */
	add 	r0, r0, #4		/* skip reserved */
	add 	r4, r4, #4		/* skip reserved */
/* save all 32 single-precision registers from ctx mem */
	mov 	r2, #32
savefpuctxt: 
	ldr 	r3, [r4], #4
	str		r3, [r0], #4
	subs 	r2, r2, #1
	bne		savefpuctxt
	/* save FPSCR from ctx mem */
	ldr		r3, [r4], #4
	str		r3, [r0], #4
	/* save FPEXC to memory */
	ldr		r3, [r4], #4
	str 	r3, [r0], #4		
	mov 	r2,#CONTEXT_MEM_LEN
	mov		r4, r12
/* save next task's context ([r1]) to the CONTEXT_MEM.
 * restore next task's context when return from ISR 
 */
restrctxt:
	ldr		r3,[r1], #4
	str		r3,[r4], #4
	subs	r2,r2, #1
	bne	restrctxt
	add 	r4, r4, #4		/* skip reserved */
	add 	r1, r1, #4		/* skip reserved */
	mov 	r2, #32
/* restore FPU context from next task's FPU context */
restrfpuctxt:
	ldr		r3, [r1], #4
	str		r3, [r4], #4
	subs	r2, r2, #1
	bne restrfpuctxt
	/* save FPSCR to ctx mem */
	ldr 	r3, [r1], #4		
	str		r3, [r4], #4
	/* save FPEXC to ctx mem */
	ldr 	r3, [r1], #4	
	str		r3, [r4], #4
	pop	{r0-r5, pc}

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
loop1:	
	ldrex	r2,[r0]
	cmp	r2,r1
	beq	loop1
	/* store r1 to [r0], r2 is result */
	strexne	r2,r1,[r0] 
	cmpne	r2,#1 
	beq	loop1
	/*lock acquired*/
	DMB
	mov	pc, lr
.global spin_lock_release
spin_lock_release:
	ldr	r1,=UNLOCKED
	DMB
	str	r1,[r0]
	mov	pc, lr
.global smp_processor_id
smp_processor_id:
	mrc	p15, 0, r0, c0, c0, 5 /* read MPIDR */
	bic	r0, r0, #0xFFFFFFFC
	mov 	pc, lr

.global read_scr
read_scr:
	mrc 	p15, 0, r0, c1, c1, 0 /* read SCR */
#if 0
	orr	r0, r0, #6 /* Monitor mode entered when IRQ, FIQ is taken*/
	mcr 	p15, 0, r0, c1, c1, 0 /* write to SCR */
	mrc 	p15, 0, r0, c1, c1, 0 /* read SCR again to check */
#endif
	mov	pc, lr
.end

/**
 * @}
 * @}
 * @}
 */