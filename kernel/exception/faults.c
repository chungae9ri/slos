#include <stdint-gcc.h>
#include <mem_layout.h>
#include <debug.h>

void platform_undefined_handler(void)
{
	print_msg("undefined cmd exception!!\r\n");
	for(;;);
}

char platform_syscall_handler(char *msg, int idx, int sys_num)
{
	char ret=0;
	switch(sys_num) {
		case 0x0: /* syscall exit */
			exit_elf(msg);
			break;

		case 0x1: /* syscal shellcmd */
			break;

		case 0x2: /* syscal write */
			msg = msg + (USER_CODE_BASE+USER_CODE_GAP*idx);
			print_msg(msg);
			break;

		case 0x3: /* syscal read */
			/*ret = uart_getc(0,1);*/
			break;

		case 0x4: /* syscal sleep*/
			put_to_sleep(msg,idx);
			break;
	}
	return ret;
}
void platform_prefetch_abort_handler(void)
{
	print_msg("prefetch abort exception!!\r\n");
	for(;;);
}
void abort(void)
{
	print_msg("data abort exception!!\r\n");
	/* here is the routine to check the page fault */
	unsigned int dfsr, dfar;
	/* read DFSR */
	asm volatile ( "mrc p15, 0, %0, c5, c0, 0" : "=r" (dfsr) ::);

	/* page fault handler should be here */
	if(dfsr & 0x07) {
		print_msg("page fault\r\n");
		/* read DFAR */
		asm volatile ("mrc p15, 0, %0, c6, c0, 0" : "=r" (dfar) ::);
	}

	for(;;);
}

