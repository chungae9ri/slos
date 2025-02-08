/**
 * @addtogroup kernel 
 * @{
 * @addtogroup kernel_arch Arch
 * @{
 * @addtogroup kernel_arch_ca9 Cortex-A9
 * @{
 * 
 * @file
 * @brief Start secondary CPU
 */

// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

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

/**
 * @}
 * @}
 * @}
 */