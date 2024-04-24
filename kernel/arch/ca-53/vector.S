// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

.section EXCEPTIONS, "ax"

.global fiq_intr_handler
.global irq_intr_handler
.global serr_intr_handler
.global synch_intr_handler

.global exceptions

exceptions:
    /* initialize VBAR_EL3.*/
    ldr x1, =vector_table_el3
    msr VBAR_EL3, x1

    /* initialize sp for EL3 */
    /*ADRP x0, #0x100000*/
    mov sp, #0x100000  
    /* configure scr for return EL1 */ 
    mrs x0, SCR_EL3
    orr x0, x0, #(1 << 10)  /* EL2 execution state is aarch64 */
    orr x0, x0, #(1 << 0)   /* EL2 is NS */
    msr SCR_EL3, x0
    mov x0, #0b01001        /* EL2 with handler stack - EL2_h */
    msr SPSR_EL3, x0

    /* elr to EL2 entry */
    adr x0, el2_entry_aarch64
    msr ELR_EL3, x0 
    eret

el2_entry_aarch64:
    /* prepare to exception return to EL1 */
    mrs x0, HCR_EL2
    orr x0, x0, #(1 << 31)  /* RW=1, EL1 execution state is aarch64 */
    msr HCR_EL2, x0
    mov x0, #0b00101        /* EL1 with handler stack - EL1_h */
    msr SPSR_EL2, x0

    /* elr to EL1 entry */
    msr SCTLR_EL1, xzr
    adr x0, el1_entry_aarch64
    ldr x1, =0x30C50838
    msr ELR_EL2, x0
    /*msr SCTLR_EL1, x1*/

    /* exception return to EL1 */
    eret

el1_entry_aarch64:
    // we can use the same vector table in this example, but in general
    // each combination of Exception level, Security state, and Execution state
    // will need a new vector table
    //ldr      x0, =vectors
    //msr      VBAR_EL1, x0

    //we must ensure that floating point register accesses are not trapped
    //since the c library for AArch64-v8A uses them
    //mov      x0, #(0x3 << 20)
    //msr      CPACR_EL1, x0
  
	mov sp, #0x200000
	b main	

.balign 0x800
/* current EL with SP0 */
vector_table_el3:
	b	synch_intr_handler

.balign 0x80
	b	irq_intr_handler

.balign 0x80
	b	fiq_intr_handler

.balign 0x80
	b	serr_intr_handler

/* current EL with SPx */
.balign 0x80
	b	synch_intr_handler

.balign 0x80
	b	irq_intr_handler

.balign 0x80
	b	fiq_intr_handler

.balign 0x80
	b	serr_intr_handler

/* lower EL using aarch64 */
.balign 0x80
	b	synch_intr_handler

.balign 0x80
	b	irq_intr_handler

.balign 0x80
	b	fiq_intr_handler

.balign 0x80
	b	serr_intr_handler

/* lower EL using aarch32 */
.balign 0x80
	b	synch_intr_handler

.balign 0x80
	b	irq_intr_handler

.balign 0x80
	b	fiq_intr_handler

.balign 0x80
	b	serr_intr_handler

/* dumb exception handler */
synch_intr_handler:
    b .
    eret

irq_intr_handler:
    b .
    eret

fiq_intr_handler:
    b .
    eret

serr_intr_handler:
    b .
    eret

.end
