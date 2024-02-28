// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

.global exit
exit:
	svc #0
loooop:
    b       loooop

.global syscmd
syscmd:
	mov r12, lr
	svc #1
	mov pc, r12

.global write
write:
	mov r12, lr
	svc #2
	mov pc, r12

.global read
read:
	mov r12, lr
	svc #3
	mov pc, r12

.global sleep
sleep:
	mov r12, lr
	svc #4
	mov pc, r12
.end
