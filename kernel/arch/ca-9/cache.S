/* 
 * Most of this routine comes from ARM Cortex-A Series Programmer's 
 * Guide v4.0 document as it is. Functional operation is not verified. 
 */

.global flush_ent_dcache

flush_ent_dcache:
	push  	{r0-r12, lr}
	/*	mov	r0, #0 */
	/*	mcr	p15, 0, r0, c8, c7, 0	 invalidate TLBs */
	bl 	clean_dcache 	 	@ clean dcache 
	pop	{r0-r12, pc}
clean_dcache:
	mrc p15, 1, r0, c0, c0, 1 	@ Read CLIDR into r0
	ands r3, r0, #0x07000000
	mov r3, r3, lsr #23 		@ Cache level value (naturally aligned)
	beq finished
	mov r10, #0
loop1:
	add r2, r10, r10, lsr #1 	@ Work out 3 x cache level
	mov r1, r0, lsr r2 		@ bottom 3 bits are the Cache type for this level
	and r1, r1, #7 			@ get those 3 bits alone
	cmp r1, #2
	blt skip 			@ no cache or only instruction cache at this level
	mcr p15, 2, r10, c0, c0, 0 	@ write CSSELR from r10
	isb 				@ ISB to sync the change to the CCSIDR
	mrc p15, 1, r1, c0, c0, 0 	@ read current CCSIDR to r1
	and r2, r1, #7 			@ extract the line length field
	add r2, r2, #4 			@ add 4 for the line length offset (log2 16 bytes)
	ldr r4, =0x3FF
	ands r4, r4, r1, lsr #3 	@ r4 is the max number on the way size (right aligned)
	clz r5, r4 			@ r5 is the bit position of the way size increment
	mov r9, r4 			@ r9 working copy of the max way size (right aligned)
loop2:
	ldr r7, =0x00007FFF
	ands r7, r7, r1, lsr #13 	@ r7 is the max num of the index size (right aligned)
loop3:
	orr r11, r10, r9, lsl r5 	@ factor in the way number and cache number into r11
	orr r11, r11, r7, lsl r2 	@ factor in the index number
	mcr p15, 0, r11, c7, c10, 2 	@ DCCSW, clean by set/way
	subs r7, r7, #1 		@ decrement the index
	bge loop3
	subs r9, r9, #1 		@ decrement the way number
	bge loop2
skip:
	add r10, r10, #2 		@ increment the cache number
	cmp r3, r10
	bgt loop1
	dsb
finished:
	mov	r10, #0			@ switch back to cache level 0
	mcr	p15, 2, r10, c0, c0, 0	@ select current cache level in cssr
	dsb	st
	isb
	bx	lr

