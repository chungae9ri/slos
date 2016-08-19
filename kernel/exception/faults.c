#include <mem_layout.h>
#include <debug.h>
#include <page_table.h>
#include <loader.h>
#include <task.h>

#define ALIGNMENT_FLT 		0x1
#define BUS_ERR_TRN_LVL1		0xc
#define BUS_ERR_TRN_LVL2		0xe
#define TRANSLATION_FLT_SEC		0x5
#define TRANSLATION_FLT_PG		0x7
#define DOMAIN_FLT_SEC		0x9
#define DOMAIN_FLT_PG		0xb
#define PERM_FLT_SEC		0xd
#define PERM_FLT_PG		0xf
#define BUS_ERR_LF_SEC		0x4
#define BUS_ERR_LF_PG		0x6
#define BUS_ERR_OTH_SEC		0x8
#define BUS_ERR_OTH_PG		0xa


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
			exit_elf(idx);
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

void abort()
{
	print_msg("data abort exception!!\r\n");
	for(;;);
}

void platform_abort_handler(void)
{
	/* here is the routine to check the page fault */
	unsigned int dfsr;
	unsigned int fault_type;
	/* read DFSR */
	asm volatile ( "mrc p15, 0, %0, c5, c0, 0" : "=r" (dfsr) ::);

	/* section/page fault handler */
	if ((dfsr & TRANSLATION_FLT_PG) == TRANSLATION_FLT_PG) {
		print_msg("section fault \n");
		handle_fault();
	} else if ((dfsr & TRANSLATION_FLT_SEC) == TRANSLATION_FLT_SEC) {
		print_msg("page fault\r\n");
		handle_fault();
	} else {
		abort();
	}
}
