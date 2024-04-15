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
    
	mov sp, #0x100000
	b main	

.balign 0x800
vector_table_el3:
	b	synch_intr_handler

.balign 0x80
	b	irq_intr_handler

.balign 0x80
	b	fiq_intr_handler

.balign 0x80
	b	serr_intr_handler

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
