.arm

.global secondary_reset
secondary_reset:
	ldr 	r0, secondary_reset_jump
	bx 	r0
.global secondary_reset_jump
secondary_reset_jump:
	/* cpu1 reset handler addr*/
	.word 	0
.global secondary_reset_jump_end
secondary_reset_jump_end:
secondary_reset_end:
