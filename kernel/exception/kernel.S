/* arm exception code */

.extern main 
.section EXCEPTIONS, "ax"
.arm
.global exceptions
exceptions :
	b reset_handler		

reset_handler:
	/*; jump to main */
	b	main	
.end
