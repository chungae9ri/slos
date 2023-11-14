.include "mem_layout.h"

.extern reset_handler_ns

.global init_monitor 
.arm
.text

/* When entering init_monitor, I, F are disabled */
init_monitor:
/* 1.1 load monitor handler address */
ldr r0, =monitor_handler
/* 1.2 set monitor vector base address */
mcr p15, 0, r0, c12, c0, 1

/* 2. save current secure context */
LDR r0, =MON_CONTEXT_MEM_S
STMFD r0!, {r4-r12}
MRS r1, cpsr
STMFD r0!, {r1, lr}

/* 3.1 change to monitor mode */
cps #MODE_MON
/* 3.2 set monitor mode stack  */
ldr r0, =MON_STACK_BASE
mov r13, r0

/*msr spsr_cxsf, #MODE_SVC*/

/* 4.switch to non secure world */
/* 4.1 read scr register */
mrc p15, 0, r0, c1, c1, 0
/* 4.2 set NS bit */
orr r0, #NS_BIT
/* 4.3 write scr register */
mcr p15, 0, r0, c1, c1, 0

/* 5. jump to normal world start */
/* 5.1 clear general purpose registers */
mov r0, #0
mov r1, #0
mov r2, #0
mov r3, #0
mov r4, #0
mov r5, #0
mov r6, #0
mov r7, #0
mov r8, #0
mov r9, #0
mov r10, #0
mov r11, #0
mov r12, #0

msr spsr_cxsf, #MODE_SVC

/* 5.2 jump to normal world start */
ldr lr, =ssbl_ns
movs pc, lr

.global monitor_handler
monitor_handler:
/* figure out source world with the NS bit */
/* save context to the source world context mem */
/* restore context from the target world context mem */
/* jump to target world lr */


