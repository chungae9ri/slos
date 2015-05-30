.global exit
exit:
	swi     #0
loooop:
    b       loooop

.global syscmd
syscmd:
	mov r12, lr
	swi	#1
	mov pc, r12

.global write
write:
	mov r12, lr
	swi      #2
	mov pc, r12

.global read
read:
	mov r12, lr
	swi      #3
	mov pc, r12

.global sleep
sleep:
	mov r12, lr
	swi      #4
	mov pc, r12
.end
