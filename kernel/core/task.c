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

#include <stdio.h>
#include <string.h>
#include <stdint-gcc.h>
#include <mem_layout.h>
#include <task.h>
#include <stdlib.h>
#include <waitq.h>
#include <runq.h>
#include <defs.h>
#include <ktimer.h>
#include <xil_printf.h>
#include <file.h>
#include <loader.h>
#include <slos_error.h>
#include <odev.h>
#include <percpu.h>

#define SVCSPSR 0x13 
#define COPROC_SRC_ADDR		0x20000000
#define COPROC_DST_ADDR		0x30000000
#define COPROC_DAT_LEN		0x10000

uint32_t show_stat = 0;
extern char inbyte(void);
extern struct task_struct *upt[MAX_USR_TASK];
extern void enable_interrupt(void);
extern void disable_interrupt(void);

void create_usr_cfs_task(char *name, 
		task_entry cfs_task, 
		uint32_t pri, 
		uint32_t appIdx)
{
	struct cfs_rq *this_runq = NULL;

#if _ENABLE_SMP_
	this_runq = __get_cpu_var(runq);
#else
	this_runq = runq;
#endif

	upt[appIdx] = forkyi(name, (task_entry)cfs_task, CFS_TASK);
	set_priority(upt[appIdx], pri);
	rb_init_node(&(upt[appIdx]->se).run_node);
	enqueue_se_to_runq(&(upt[appIdx]->se), true);
	this_runq->cfs_task_num++;
}

void create_cfs_task(char *name, task_entry cfs_task, uint32_t pri)
{
	struct task_struct *temp = NULL;
	struct cfs_rq *this_runq = NULL;

#if _ENABLE_SMP_
	this_runq = __get_cpu_var(runq);
#else
	this_runq = runq;
#endif

	temp = forkyi(name, (task_entry)cfs_task, CFS_TASK);
	set_priority(temp, pri);
	rb_init_node(&temp->se.run_node);
	enqueue_se_to_runq(&temp->se, true);
	this_runq->cfs_task_num++;
}

void create_rt_task(char *name, task_entry handler, uint32_t dur)
{
	static uint32_t rt_timer_idx = 0;
	struct task_struct *temp;

	temp = forkyi(name, (task_entry)handler, RT_TASK);
	temp->timeinterval = dur;
	temp->state = TASK_RUNNING;

	disable_interrupt();
	create_rt_timer(temp, dur, rt_timer_idx++, NULL);
	enable_interrupt();
}

void create_oneshot_task(char *name, task_entry handler, uint32_t dur)
{
	static uint32_t oneshot_timer_idx = 0;
	struct task_struct *temp;

	temp = forkyi(name, (task_entry)handler, ONESHOT_TASK);
	temp->timeinterval = dur;
	temp->state = TASK_RUNNING;
	create_oneshot_timer(temp, dur, oneshot_timer_idx++, NULL);
}

uint32_t rt_worker2(void)
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
			xil_printf("I am rt worker2 j: %d\n", j);
		}
		for (i = 0; i < 500; i++) {
			j++;
		}

		if (show_stat) {
			xil_printf("I am rt worker2 j: %d\n", j);
		}
		j = 0;
		this_current->done = 1;
		/* should yield after finish current work */
		yield();
	}
	return ERR_NO;
}

uint32_t rt_worker1(void)
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
			xil_printf("I am rt worker1 j: %d\n", j);	
		}
		for (i = 0; i < 1000; i++) {
			j++;
		}

		if (show_stat) {
			xil_printf("I am rt worker1 j: %d\n", j);	
		}
		j = 0;
		this_current->done = 1;
		/* should yield after finish current work */
		yield();
	}
	return ERR_NO;
}

#include <dma.h>

uint32_t oneshot_worker(void)
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

		xil_printf("I am oneshot_worker\n");

		/* should yield after finish current work */
		yield();
	}

	return ERR_NO;
}


uint32_t cfs_dummy(void )
{
	uint32_t cnt;
	int i; 

	cnt = 0;
	while (1) {
		cnt++;
		if (cnt > 1000000)
			cnt = 0;
		xil_printf("dummy cfs worker cnt: %d\n", cnt);
		for (i = 0; i < 1000000; i++);
	}
}

#define TEST_KMALLOC_SZ 	1024 * 1024	
uint32_t cfs_worker1(void )
{
	uint8_t *pc;
	uint8_t t;
	int i;

	xil_printf("I am cfs_worker1....\n");

	pc = NULL;
	while (1) {
		if (show_stat) {
			xil_printf("cfs_worker1 is running....\n");
		}

		pc = (uint8_t *)kmalloc(sizeof(uint8_t) * TEST_KMALLOC_SZ);
		if (pc != NULL) {
			for (i = 0; i < TEST_KMALLOC_SZ; i++) {
				t = (uint8_t)(i % 256);
				pc[i] = t;
			}
		} 
		kfree((uint32_t)pc);
		pc = NULL;
	}

	return ERR_NO;
}

#define FILE_TEST_LEN 1024	

uint32_t cfs_worker2(void)
{
	char buf[FILE_TEST_LEN];
	char temp[FILE_TEST_LEN];
	int i;
	struct file *fp;

	xil_printf("I am cfs_worker2....\n");

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
	xil_printf("file test : ");
	for (i = 0; i < FILE_TEST_LEN; i++) {
		if (buf[i] != temp[i]) {
			xil_printf("fail!!\n");
			break;
		}
	}
	if (i == FILE_TEST_LEN) {
		xil_printf("pass!!\n");
	} else {
		xil_printf("fail!! i: %d\n", i);
	}

	close_file(fp);
	fp = NULL;

	while (1) {
		if (show_stat) {
			xil_printf("cfs_worker2 running....\n");
		}
	}
	return ERR_NO;
}

/* not used. shell cmd start dma does this. deprecated.*/
#if 0 
uint32_t cfs_worker3(void )
{
	uint8_t *psrc;
	int i, j;

	xil_printf("I am cfs_worker3....\n");

	i = 0;
	while (1) {
		psrc = (uint8_t *)COPROC_SRC_ADDR;
		for (j = 0; j < COPROC_DAT_LEN; j++) {
			psrc[j] = (uint8_t)(j % 256);
		}

		if (show_stat) {
			xil_printf("cfs_worker3 is running....\n");
		}

		if (i == 0) {
			set_dma_work(COPROC_SRC_ADDR, COPROC_DST_ADDR, COPROC_DAT_LEN);
			start_dma(NULL);
			i++;
		}
	}

	return ERR_NO;
}
#endif

extern void mdelay(unsigned msecs);

uint32_t workq_worker(void)
{
	int i, j, enq_idx, deq_idx;
	struct worker *this_qworker;

#if _ENABLE_SMP_
	this_qworker = __get_cpu_var(qworker);
#else
	this_qworker = qworker;
#endif

	while (1) {
		/*xil_printf("%s deq_idx:%d\n", __func__, qworker.deq_idx);*/
		enq_idx = this_qworker->enq_idx;
		deq_idx = this_qworker->deq_idx;
		
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

		/*xil_printf("qworker_deq_idx: %d\n", this_qworker->deq_idx);*/

		for (i = 0; i < 10000; i++);
	}

	return ERR_NO;
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

	return ERR_NO;
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
	sprintf(worker_name, "workq_worker:%d", (int)cpuid); 
	create_cfs_task(worker_name, workq_worker, 4);
}

void create_cfs_workers(void)
{
#if 1
	/*create_cfs_task("cfs_worker1", cfs_dummy, 8);*/
	/*create_cfs_task("cfs_worker2", cfs_dummy, 4);*/
	/*create_cfs_task("cfs_worker3", cfs_worker3, 8);*/
	/*create_cfs_task("cfs_worker4", cfs_worker4, 4);*/
#else
	/*create_cfs_task("cfs_worker1", cfs_worker1, 8);*/
	create_cfs_task("cfs_worker2", cfs_worker2, 4);
	create_cfs_task("cfs_worker3", cfs_worker3, 8);
	/*create_cfs_task("cfs_worker4", cfs_worker4, 4);*/
#endif
}

void create_rt_workers(void)
{
	create_rt_task("rt_worker1", rt_worker1, 120);
	create_rt_task("rt_worker2", rt_worker2, 125);
}
#define CMD_LEN		32	


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

	xil_printf("I am shell\n");

	while (1) {
		i = 0;
		xil_printf("shell > ");

		do {
			byte = inbyte();
			outbyte(byte);
			cmdline[i++] = byte;

		} while(byte != '\n' && byte != '\r' && i < CMD_LEN);

		cmdline[--i] = '\0';

		xil_printf("\n");
		if (cmdline[0] == '\0' || !strcmp(cmdline, "help")) {
			xil_printf("taskstat, whoami, hide whoami\n");
			xil_printf("cfs task, rt task, oneshot task\n");
			xil_printf("sleep, run, start dma \n");
			xil_printf("apprun, start cs, set cs\n");
		} else if (!strcmp(cmdline, "taskstat")) {
			print_task_stat();
		} else if (!strcmp(cmdline, "whoami")) {
			show_stat = 1;
		} else if (!strcmp(cmdline, "hide whoami")) {
			show_stat = 0;
		} else if (!strcmp(cmdline, "cfs task")) {
			xil_printf("add cfs task \n");
			create_cfs_workers();
		} else if (!strcmp(cmdline, "rt task")) {
			xil_printf("add rt task \n");
			create_rt_workers();
		} else if (!strcmp(cmdline, "oneshot task")) {
			xil_printf("add oneshottask \n");
			create_oneshot_task("oneshot_task", oneshot_worker, 1000);
		} else if (!strcmp(cmdline, "sleep")) {
			xil_printf("input task pid: ");
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
				dequeue_se_to_wq(&pwait_task->se, true);
			} else {
				xil_printf("task %d is not in runq\n", pid);
			}

		} else if (!strcmp(cmdline, "run")) {
			xil_printf("input task pid: ");
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
				enqueue_se_to_runq(&pwait_task->se, true);
			} else {
				xil_printf("task %d is not in runq\n", pid);
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
		} else if (!strcmp(cmdline, "sgi")) {
			uint32_t sgir = 0x0002000F;
			*(volatile uint32_t *)(0xF8F01F00) = sgir;
		}
		else {
			xil_printf("I don't know.... ^^;\n");
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

	temp = forkyi("shell", (task_entry)shell, CFS_TASK);
	set_priority(temp, 2);
	rb_init_node(&temp->se.run_node);
	enqueue_se_to_runq(&temp->se, true);
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
	pt->type = CFS_TASK;
	pt->missed_cnt = 0;
	pt->pid = 0;
	pt->preempted = 0;
	set_priority(pt, 16);
#if _ENABLE_SMP_
	__get_cpu_var(idle_task) = __get_cpu_var(current) = __get_cpu_var(first) = __get_cpu_var(last) = pt;
	pthis_task_created_num = (uint32_t *) __get_cpu_var_addr(task_created_num);
#else
	idle_task = current = first = last = pt;
	pthis_task_created_num = &task_created_num;
#endif
	*pthis_task_created_num = 1;
}
