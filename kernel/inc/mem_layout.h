#ifdef __ASSEMBLY__
.set KERNEL_CODE_BASE			, 0x8000
.set KERNEL_EXCEPTION_START		, 0x8000
.set USER_CODE_BASE			, 0x1600000 /* user task code base */
.set USER_CODE_GAP			, 0x10000  /* 128KB user code gap */
.set SYS_STACK_BASE			, 0x800000 /* 8M */
.set SVC_STACK_BASE			, 0x880000 /* 8.5M */
.set IRQ_STACK_BASE			, 0x900000 /* 9M */
.set FIQ_STACK_BASE			, 0x980000 /* 9.5M */
.set ABT_STACK_BASE			, 0xA00000 /* 10M */
.set UNDEF_STACK_BASE			, 0xA80000 /* 10.5M */
.set KERNEL_TASK_STACK_BASE		, 0xB00000 /* 11M, but not used since all tasks in kernel are in SVC */
.set USER_TASK_STACK_BASE		, 0xB80000 /* 11.5M for HelloWorld */
.set TASK_STACK_GAP			, 0x8000 /* 32k */
/*.set HEAP_BASE			, 0x01000000
.set HEAP_SIZE				, 0x500000 */
.set SCRATCH_BASE			, 0x2000000 /* 0x32M */
#else
#define KERNEL_CODE_BASE		0x8000
#define KERNEL_EXCEPTION_START	0x8000
#define USER_CODE_BASE		0x1600000 /* user task code base */
#define USER_CODE_GAP		0x10000  /* 128KB user code gap */
#define SYS_STACK_BASE		0x800000 /* 8M */
#define SVC_STACK_BASE		0x880000 /* 8.5M */
#define IRQ_STACK_BASE		0x900000 /* 9M */
#define FIQ_STACK_BASE		0x980000 /* 9.5M */
#define ABT_STACK_BASE		0xA00000 /* 10M */
#define UNDEF_STACK_BASE		0xA80000 /* 10.5M */
#define KERNEL_TASK_STACK_BASE	0xB00000 /* 11M, but not used since all tasks in kernel are in SVC */
#define USER_TASK_STACK_BASE	0xB80000 /* 11.5M for HelloWorld */
#define TASK_STACK_GAP		0x8000 /* 32k */
/*.set HEAP_BASE			, 0x1000000
.set HEAP_SIZE				, 0x500000 */
#define SCRATCH_BASE		0x2000000 /* 0x32M */
#endif
