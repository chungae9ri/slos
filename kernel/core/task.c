/*
  kernel/core/task.c process management
  (C) 2018 Kwangdo Yi <kwangdo.yi@gmail.com>
 
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, see <http://www.gnu.org/licenses/>
*/

#include <stdint.h>
#include <mem_layout.h>
#include <task.h>
#include <waitq.h>
#include <runq.h>
#include <defs.h>
#include <ktimer.h>
#include <printk.h>
#include <loader.h>
#include <error.h>
#include <odev.h>
#include <percpu.h>
#include <mailbox.h>
#include <timer.h>
#include <rwbyte.h>
#include <string.h>
#include <dma.h>
#include <ramdisk_io.h>

#define SVCSPSR 			0x13 
#define COPROC_SRC_ADDR		0x10000000
#define COPROC_DAT_LEN		0x1000
#define COPROC_DST_ADDR		(COPROC_SRC_ADDR + COPROC_DAT_LEN)
#define ICDSGIR				0xF8F01F00
#define CMD_LEN				32	

uint32_t show_stat = 0;
extern struct task_struct *upt[MAX_USR_TASK];
extern void enable_interrupt(void);
extern void disable_interrupt(void);

#ifdef LITTLEFS
#include <lfs.h>

static struct lfs_config cfg;
static lfs_t lfs;
static lfs_file_t lfs_file;

static int lfs_api_read(const struct lfs_config *c, lfs_block_t block,
			lfs_off_t off, void *buffer, lfs_size_t size)
{
	size_t offset = block * c->block_size + off;

	return io_ops.read(offset, size, buffer);
}

static int lfs_api_prog(const struct lfs_config *c, lfs_block_t block,
			lfs_off_t off, const void *buffer, lfs_size_t size)
{
	size_t offset = block * c->block_size + off;

	return io_ops.write(offset, size, buffer);
}

static int lfs_api_erase(const struct lfs_config *c, lfs_block_t block)
{
	return io_ops.erase_page(block);
}

int littlefs_task(void)
{
	int32_t ret;

	cfg.read_size = 1;
	cfg.prog_size = 1;
	cfg.block_size = RAMDISK_PAGE_SIZE;
	cfg.block_count = RAMDISK_PAGE_NUM;
	cfg.block_cycles = 500;
	cfg.cache_size = 32;
	cfg.lookahead_size = 32;

	cfg.read = lfs_api_read;
	cfg.prog = lfs_api_prog;
	cfg.erase = lfs_api_erase;

	ret = lfs_mount(&lfs, &cfg);
    /* reformat if we can't mount the filesystem
	 * this should only happen on the first boot
	 */
    if (ret) {
        lfs_format(&lfs, &cfg);
        lfs_mount(&lfs, &cfg);
    }

    /* release any resources we were using */
    lfs_unmount(&lfs);

	return NO_ERR;
}
#endif

static uint32_t oneshot_worker(void)
{
	int i, j = 0;
	while (1) {
		/* do some one time work here */
#if 0
		if (i == 0) {
			set_dma_work(0x20000000, 0x30000000, 0x1000);
			start_dma();
		}
#endif

		for (i = 0; i < 1000; i++) {
			j++;
		}
		j = 0;

		printk("I am oneshot_worker\n");

		/* should yield after finish current work */
		yield();
	}

	return NO_ERR;
}

static uint32_t rt_worker1(void)
{
	int i, j = 0;
	struct task_struct *this_current = NULL;

#if _ENABLE_SMP_
		this_current = __get_cpu_var(current);
#else
		this_current = current;
#endif
	while (1) {
		/* do some real time work here */
		this_current->done = 0;
		if (show_stat) {
			printk("I am rt worker1 j: 0x%x\n", j);	
		}
		for (i = 0; i < 1000; i++) {
			j++;
		}

		j = 0;
		this_current->done = 1;
		/* should yield after finish current work */
		yield();
	}
	return NO_ERR;
}

static uint32_t rt_worker2(void)
{
	int i, j = 0;
	struct task_struct *this_current = NULL;
#if _ENABLE_SMP_
	this_current = __get_cpu_var(current);
#else
	this_current = current;
#endif
	while (1) {
		/* do some real time work here */
		this_current->done = 0;
		if (show_stat) {
			printk("I am rt worker2 j: 0x%x\n", j);
		}
		for (i = 0; i < 500; i++) {
			j++;
		}

		j = 0;
		this_current->done = 1;
		/* should yield after finish current work */
		yield();
	}
	return NO_ERR;
}

static uint32_t cfs_dummy1(void )
{
	uint32_t cnt;

	cnt = 0;
	while (1) {
		cnt++;
		if (cnt > 1000000)
			cnt = 0;
		printk("dummy1 cfs worker cnt: 0x%x\n", cnt);
		msleep(1000);
	}

	return NO_ERR;
}

static uint32_t cfs_dummy2(void )
{
	uint32_t cnt;

	cnt = 0;
	while (1) {
		cnt++;
		if (cnt > 1000000)
			cnt = 0;
		printk("dummy 2 cfs worker cnt: 0x%x\n", cnt);
		msleep(1000);
	}

	return NO_ERR;
}

static uint32_t cfs_dummy3(void )
{
	uint32_t cnt;

	cnt = 0;
	while (1) {
		cnt++;
		if (cnt > 1000000)
			cnt = 0;
		printk("dummy 3 cfs worker cnt: 0x%x\n", cnt);
		msleep(1000);
	}

	return NO_ERR;
}

#define TEST_KMALLOC_SZ 	1024 * 1024
static uint32_t test_mem(void)
{
	uint32_t cnt;
	uint8_t *pc;
	int i;

	printk("I am %s worker\n", __func__);

	pc = NULL;
	/* demanding pages */
	pc = (uint8_t *)kmalloc(sizeof(uint8_t) * TEST_KMALLOC_SZ);
	if (pc != NULL) {
		/* write pattern */
		for (i = 0; i < TEST_KMALLOC_SZ; i++)
			pc[i] = (uint8_t)(i % 256);

		/* check pattern */
		for (i = 0; i < TEST_KMALLOC_SZ; i++) {
			if (pc[i] != (uint8_t)(i % 256)) {
				printk("demaning page test fail, value: 0x%x at 0x%x\n", pc[i], i);
				break;
			}
		}

		if (i == TEST_KMALLOC_SZ) {
			printk("test mem pass\n");
			/* free memory */
			kfree((uint32_t)pc);
			pc = NULL;
		}
	}

	cnt = 0;
	while (1) {
		cnt++;
		if (cnt > 1000000)
			cnt = 0;

		printk("test_mem cfs dummy worker cnt: 0x%x\n", cnt);
		msleep(1000);
	}

	return NO_ERR;
}

#ifdef FILE_SYSTEM
#define FILE_TEST_LEN 1024	

static uint32_t cfs_worker2(void)
{
	char buf[FILE_TEST_LEN];
	char temp[FILE_TEST_LEN];
	int i;
	struct file *fp;

	printk("I am cfs_worker2....\n");

	/*
	buf = (char *)kmalloc(sizeof(char) * FILE_TEST_LEN);
	temp = (char *)kmalloc(sizeof(char) * FILE_TEST_LEN);
	*/

	fp = open_file("test");
	for (i = 0; i < FILE_TEST_LEN; i++) 
		buf[i] = i % 256;
	write(fp, FILE_TEST_LEN, buf);
	reset(fp);
	read(fp, FILE_TEST_LEN, temp);
	printk("file test : ");
	for (i = 0; i < FILE_TEST_LEN; i++) {
		if (buf[i] != temp[i]) {
			printk("fail!!\n");
			break;
		}
	}
	if (i == FILE_TEST_LEN) {
		printk("pass!!\n");
	} else {
		printk("fail!! i: %x\n", i);
	}

	close_file(fp);
	fp = NULL;

	while (1) {
		if (show_stat) {
			printk("cfs_worker2 running....\n");
		}
	}
	return NO_ERR;
}
#endif

static uint32_t workq_worker(void)
{
	uint32_t cpuid;
	int i, j, enq_idx, deq_idx;
	struct worker *this_qworker;
	struct task_struct *this_current;

#if _ENABLE_SMP_
	this_qworker = __get_cpu_var(qworker);
	this_current = __get_cpu_var(current);
#else
	this_qworker = qworker;
	this_current = current;
#endif

	cpuid = smp_processor_id();

	while (1) {
		/* Woken up */
		enq_idx = this_qworker->enq_idx;
		deq_idx = this_qworker->deq_idx;
		printk("cpu 0x%x qworker enq_idx: 0x%x, deq_idx: 0x%x\n", cpuid, enq_idx, deq_idx);
		
		/* enq_idx is wrapped around */
		if (enq_idx < deq_idx)
			enq_idx += MAX_WORKQ; 
		/* enq_idx is no atomic, but it's ok */
		for (i = deq_idx; i < enq_idx; i++) {
			j = i % MAX_WORKQ;
			this_qworker->wkq[j].func(this_qworker->wkq[j].arg);
		}

		if (enq_idx >= MAX_WORKQ)
			enq_idx -= MAX_WORKQ;

		this_qworker->deq_idx = enq_idx;

		/* Fall back into waiting state */
		dequeue_se_to_wq(&this_current->se);
		yield();
	}

	return NO_ERR;
}

static void create_cfs_workers(void)
{
	static int cfs_worker_num = 0;

	if (cfs_worker_num == 0)
		create_cfs_task("cfs_worker1", cfs_dummy1, 8);
	else if (cfs_worker_num == 1)
		create_cfs_task("cfs_worker2", cfs_dummy2, 4);
	else if (cfs_worker_num == 2)
		create_cfs_task("cfs_worker3", cfs_dummy3, 4);
	else 
		printk("cfs worker number limit\n");

	cfs_worker_num++;
}

static void create_rt_workers(void)
{
	create_rt_task("rt_worker1", rt_worker1, 120);
	create_rt_task("rt_worker2", rt_worker2, 125);
}

struct task_struct* create_oneshot_task(char *name, task_entry handler, uint32_t msec)
{
	struct task_struct *task= NULL;
	uint32_t tc = 0;

	task= forkyi(name, (task_entry)handler, ONESHOT_TASK, 0);
	task->timeinterval = msec;
	task->state = TASK_RUNNING;
	tc = get_ticks_per_sec() / 1000 * msec;
	create_oneshot_timer(task, tc, NULL);

	return task;
}

struct task_struct *create_usr_cfs_task(char *name,
										task_entry cfs_task,
										uint32_t pri,
										uint32_t app_idx)
{
	struct cfs_rq *this_runq = NULL;

#if _ENABLE_SMP_
	this_runq = __get_cpu_var(runq);
#else
	this_runq = runq;
#endif

	upt[app_idx] = forkyi(name, (task_entry)cfs_task, CFS_TASK, pri);
	rb_init_node(&(upt[app_idx]->se).run_node);
	enqueue_se_to_runq(&(upt[app_idx]->se));
	this_runq->cfs_task_num++;

	return upt[app_idx];
}

struct task_struct* create_cfs_task(char *name, task_entry cfs_task, uint32_t pri)
{
	struct task_struct *task = NULL;
	struct cfs_rq *this_runq = NULL;

#if _ENABLE_SMP_
	this_runq = __get_cpu_var(runq);
#else
	this_runq = runq;
#endif

	task = forkyi(name, (task_entry)cfs_task, CFS_TASK, pri);
	rb_init_node(&task->se.run_node);
	enqueue_se_to_runq(&task->se);
	this_runq->cfs_task_num++;

	return task;
}

struct task_struct* create_rt_task(char *name, task_entry handler, uint32_t msec)
{
	struct task_struct *task;

	task= forkyi(name, (task_entry)handler, RT_TASK, 0);
	task->timeinterval = msec;
	task->state = TASK_RUNNING;

	disable_interrupt();
	create_rt_timer(task, msec, NULL);
	enable_interrupt();

	return task;
}

void wakeup_workq_worker(void)
{
	struct worker *this_qworker;
	struct task_struct *this_task;
#if _ENABLE_SMP_
	this_qworker = __get_cpu_var(qworker);
	this_task = this_qworker->task;
#else
	this_qworker = qworker;
	this_task = this_qworker->task;
#endif

	enqueue_se_to_runq(&this_task->se);
}

uint32_t enqueue_workq(void (*func)(void *), void *arg) 
{
	struct worker *this_qworker;
#if _ENABLE_SMP_
	this_qworker = __get_cpu_var(qworker);
#else
	this_qworker = qworker;
#endif
	this_qworker->wkq[this_qworker->enq_idx].func = func;
	this_qworker->wkq[this_qworker->enq_idx].arg = arg;

	if (this_qworker->enq_idx == MAX_WORKQ - 1) {
		this_qworker->enq_idx = 0;
	} else {
		this_qworker->enq_idx++;
	}

	return NO_ERR;
}

void create_workq_worker(void)
{
	char worker_name[16];
	uint32_t cpuid;
	struct worker *this_qworker;
#if _ENABLE_SMP_
	__get_cpu_var(qworker) = (struct worker *)kmalloc(sizeof(struct worker));
	this_qworker = __get_cpu_var(qworker);
#else
	qworker = (struct worker *)kmalloc(sizeof(struct worker));
	this_qworker = qworker;
#endif
	this_qworker->enq_idx = 0;
	this_qworker->deq_idx = 0;

	cpuid = smp_processor_id();
	sprintk(worker_name, "workq_worker:%x", (int)cpuid); 
	this_qworker->task = create_cfs_task(worker_name, workq_worker, 4);
}

void shell(void)
{
	char byte;
	char cmdline[CMD_LEN];
	uint8_t *psrc;
	int i, j, pid;
	struct task_struct *pwait_task;
	struct list_head *next_lh = NULL;
	struct cfs_rq *this_runq = NULL;

#if _ENABLE_SMP_
	this_runq = __get_cpu_var(runq);
#else
	this_runq = runq;
#endif

	show_stat = 0;

	printk("I am shell\n");

	while (1) {
		i = 0;
		printk("shell > ");

		do {
			byte = inbyte();
			outbyte(byte);
			cmdline[i++] = byte;

		} while(byte != '\n' && byte != '\r' && i < CMD_LEN);

		cmdline[--i] = '\0';

		printk("\n");
		if (cmdline[0] == '\0' || !strcmp(cmdline, "help")) {
			printk("apprun             : run user application in the ramdisk\n");
			printk("cfs task           : create and run test cfs tasks\n");
			printk("oneshot task       : create and run test oneshot task\n");
			printk("rt task            : create and run test rt tasks\n");
			printk("run                : wakeup and run a task with pid\n");
			printk("sgi                : generate sgi interrupt to cpu1\n");
			printk("sleep              : sleep a task with pid\n");
			printk("start cs           : start outstream consumer hw\n");
			printk("start dma          : start dma task\n");
			printk("taskstat           : show task statistics\n");
			printk("test mem           : run memory test task\n");
			printk("whoami, hide whoami: show or hide printing current task name\n");
		} else if (!strcmp(cmdline, "taskstat")) {
			print_task_stat(NULL);
#if _ENABLE_SMP_
			/* show cpu 1 taskstat */
			enum letter_type letter = TASK_STAT;
			push_mail(letter);
			/* TargetListFilter: b[25:24] 2b00 send the interrupt to the 
			 *  		     CPU interfaces specified in the CPU TargetList field
			 * CPUTargetList: b[23:16] 2b00000010 CPU 1 CPU interface
			 * SATT: b[15] 2b0 only if SGI is configured as Secure on that interface
			 * SBZ: b[14:4]
			 * SGIINTID: b[3:0] 0xF interrupt ID of SGI to send to the 
			 *           specified CPU interface
			 */
			uint32_t sgir = 0x0002000F;
			*(uint32_t *)(ICDSGIR) = sgir;
#endif
		} else if (!strcmp(cmdline, "whoami")) {
			show_stat = 1;
		} else if (!strcmp(cmdline, "hide whoami")) {
			show_stat = 0;
		} else if (!strcmp(cmdline, "cfs task")) {
			printk("add cfs task \n");
			create_cfs_workers();
		} else if (!strcmp(cmdline, "rt task")) {
			printk("add rt task \n");
			create_rt_workers();
		} else if (!strcmp(cmdline, "oneshot task")) {
			printk("add oneshottask \n");
			create_oneshot_task("oneshot_task", oneshot_worker, 1000);
		} else if (!strcmp(cmdline, "sleep")) { 
			printk("input task pid: "); 
			byte = inbyte(); 
			outbyte(byte); 
			outbyte('\n');
			pid = byte - '0';
#if _ENABLE_SMP_
			next_lh = &(((struct task_struct*)(__get_cpu_var(first)))->task);
#else
			next_lh = &first->task;
#endif
			for (j = 0; j < this_runq->cfs_task_num; j++) {
				pwait_task = (struct task_struct *)to_task_from_listhead(next_lh);
				if (pwait_task->pid == pid) break;
				next_lh = next_lh->next;
			}
			if (j < this_runq->cfs_task_num && pwait_task->state == TASK_RUNNING) {
				dequeue_se_to_wq(&pwait_task->se);
			} else {
				printk("task 0x%x is not in runq\n", pid);
			}

		} else if (!strcmp(cmdline, "run")) {
			printk("input task pid: ");
			byte = inbyte();
			outbyte(byte);
			outbyte('\n');
			pid = byte - '0';

#if _ENABLE_SMP_
			next_lh = &(((struct task_struct*)(__get_cpu_var(first)))->task);
#else
			next_lh = &first->task;
#endif
			for (j = 0; j < this_runq->cfs_task_num; j++) {
				pwait_task = (struct task_struct *)to_task_from_listhead(next_lh);
				if (pwait_task->pid == pid) break;
				next_lh = next_lh->next;
			}
			if (j < this_runq->cfs_task_num && pwait_task->state == TASK_WAITING) {
				enqueue_se_to_runq(&pwait_task->se);
			} else {
				printk("task 0x%x is not in runq\n", pid);
			}
		} else if (!strcmp(cmdline, "apprun")) {
			load_ramdisk_app(0);
		} else if (!strcmp(cmdline,"start cs")) {
			start_consumer();
		} else if (!strcmp(cmdline, "stop cs")) {
			stop_consumer();
		} else if (!strcmp(cmdline, "start dma")) {
			psrc = (uint8_t *)COPROC_SRC_ADDR;
			for (j = 0; j < COPROC_DAT_LEN; j++) {
				psrc[j] = (uint8_t)(j % 256);
			}

			set_dma_work(COPROC_SRC_ADDR, COPROC_DST_ADDR, COPROC_DAT_LEN);
			start_dma(NULL);
		} 
#if _ENABLE_SMP_
		else if (!strcmp(cmdline, "sgi")) {
			enum letter_type letter = TASK_ODEV;
			push_mail(letter);
			uint32_t sgir = 0x0002000F;
			*(uint32_t *)(ICDSGIR) = sgir;
		}
#endif
		else if (!strcmp(cmdline,"test mem")) {
			create_cfs_task("test_mem", test_mem, 4);
		}
		else {
			printk("I don't know.... ^^;\n");
		}
	}
}

void init_shell(void)
{
	struct task_struct *temp = NULL;
	struct cfs_rq *this_runq = NULL;

#if _ENABLE_SMP_
	this_runq = __get_cpu_var(runq);
#else
	this_runq = runq;
#endif

	temp = forkyi("shell", (task_entry)shell, CFS_TASK, 2);
	rb_init_node(&temp->se.run_node);
	enqueue_se_to_runq(&temp->se);
	this_runq->cfs_task_num++;
}
void init_idletask(void)
{
	uint32_t cpuid = 0;
	uint32_t *pthis_task_created_num;
	struct task_struct *pt = (struct task_struct *)kmalloc(sizeof(struct task_struct));

	cpuid = smp_processor_id();
	if (cpuid == 0)
		strcpy(pt->name, "idle task");
	else 
		strcpy(pt->name, "idle task secondary");

	pt->task.next = NULL;
	pt->task.prev = NULL;
	pt->yield_task = NULL;
	/*pt->se.ticks_vruntime = 0LL;*/
	pt->se.ticks_consumed = 0LL;
	pt->se.jiffies_vruntime = 0L;
	pt->se.jiffies_consumed = 0L;
	pt->se.priority = 16;
	pt->se.pri_div_shift = 4;
	pt->type = CFS_TASK;
	pt->missed_cnt = 0;
	pt->pid = cpuid;
	pt->preempted = 0;
	pt->state = TASK_RUNNING;
#if _ENABLE_SMP_
	__get_cpu_var(idle_task) = __get_cpu_var(current) = __get_cpu_var(first) = __get_cpu_var(last) = pt;
	pthis_task_created_num = (uint32_t *) __get_cpu_var_addr(task_created_num);
#else
	idle_task = current = first = last = pt;
	pthis_task_created_num = &task_created_num;
#endif
	*pthis_task_created_num = 1;
}
