.section EXCEPTIONS, "ax"
.global exceptions

exceptions:
	mov sp, #0x100000
	b main
