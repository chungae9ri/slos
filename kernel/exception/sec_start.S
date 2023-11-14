/* arm exception code */

.include "definitions.h"
.extern main 
.section EXCEPTIONS, "ax"
.arm
.global exceptions

exceptions:
	b reset_handler		

reset_handler:
	/*set secure VBAR (banked) with 0x100000 */
	ldr	r0, =0x100000
	mcr	p15,0,r0,c12,c0,0 

	 /*change to supervisor*/
	msr	CPSR_c, #MODE_SVC | I_BIT | F_BIT
	
	/* setup svc stack*/
	ldr	r0, =0x200000
	mov	r13, r0

	/*; Switch to undefined mode and setup the undefined mode stack*/
	msr CPSR_c, #MODE_UND | I_BIT | F_BIT
	ldr r0, =0x201000
	mov r13, r0

	/* Switch to abort mode and setup the abort mode stack*/
	msr CPSR_c, #MODE_ABT | I_BIT | F_BIT
	ldr	r0, =0x202000
	mov r13, r0

	/* Switch to SYS mode and setup the SYS mode stack*/
	msr CPSR_c, #MODE_SYS | I_BIT | F_BIT
	ldr	r0, =0x203000
	mov r13, r0

	/* Switch to IRQ mode and setup the IRQ mode stack*/
	msr CPSR_c, #MODE_IRQ | I_BIT | F_BIT
	ldr	r0, =0x204000
	mov r13, r0

	/* Switch to FIQ mode and setup the FIQ mode stack*/
	msr CPSR_c, #MODE_FIQ | I_BIT | F_BIT
	ldr	r0, =0x205000
	mov r13, r0

	/* Return to supervisor mode and enable irq, fiq */
	msr CPSR_c, #MODE_SVC
	/* jump to monitor mode start */
	b init_monitor

.end
