#ifdef __ASSEMBLY__
.set KERNEL_CODE_BASE			, 0x100000
.set KERNEL_EXCEPTION_START		, 0x100000
.set USER_CODE_BASE			, 0x1600000 /* user task code base */
.set USER_CODE_GAP			, 0x10000  /* 128KB user code gap */
.set KERNEL_END				, 0x400000
.set PGT_START_BASE			, 0x3FC000 /* page table base  4KB * 4 frames */
.set PGD_START_BASE			, 0x3F8000 /* page directory base 8 * 4KB frames */
.set SVC_STACK_BASE			, 0x3F7FFC /* SVC_STACK_BASE ~ 4MB-32KB */
.set SYS_STACK_BASE			, 0x327FFC /* 3M + 32KB*5 */
.set IRQ_STACK_BASE			, 0x31FFFC /* 3M + 32KB*4 */
.set FIQ_STACK_BASE			, 0x317FFC /* 3M + 32KB*3 */
.set ABT_STACK_BASE			, 0x30FFFC /* 3M + 32KB*2 */
.set UNDEF_STACK_BASE			, 0x307FFC /* 3M + 32KB */
.set TASK_STACK_GAP			, 0x8000 /* 32k */
.set SCRATCH_BASE			, 0x2000000 /* 0x32M */
#else
#define KERNEL_CODE_BASE		0x100000
#define KERNEL_EXCEPTION_START		0x100000
#define USER_CODE_BASE			0x1600000 /* user task code base */
#define USER_CODE_GAP			0x10000  /* 128KB user code gap */
#define KERNEL_END			0x400000
#define PGT_START_BASE			0x3FC000 /* page table base  4KB * 4 frames */
#define PGD_START_BASE			0x3F8000 /* page directory base 4KB * 4 frames */
#define SVC_STACK_BASE			0x3F7FFC /* SVC_STACK_BASE ~ 4MB */
#define SYS_STACK_BASE			0x327FFC /* 3M + 32KB*5 */
#define IRQ_STACK_BASE			0x31FFFC /* 3M + 32KB*4 */
#define FIQ_STACK_BASE			0x317FFC /* 3M + 32KB*3 */
#define ABT_STACK_BASE			0x30FFFC /* 3M + 32KB*2 */
#define UNDEF_STACK_BASE		0x307FFC /* 3M + 32KB */
#define TASK_STACK_GAP			0x8000 /* 32k */
#define SCRATCH_BASE			0x2000000 /* 0x32M */
#endif
