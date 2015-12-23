# Copyright (c) 2012-2013, The Linux Foundation. All rights reserved.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 and
# only version 2 as published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

import sys
import re
import os
import struct
import datetime
import array
import string
import bisect
import traceback
from subprocess import *
from optparse import OptionParser
from optparse import OptionGroup
from struct import unpack
from ctypes import *
from tempfile import *
from print_out import *


workqueue_offsets_3_0 = [
    ("((struct work_struct *)0x0)", "func", 0, 0),
    ("((struct work_struct *)0x0)", "entry", 0, 0),
    ("((struct task_struct *)0x0)", "comm", 0, 0),
    ("((struct worker *)0x0)", "entry", 0, 0),
    ("((struct worker *)0x0)", "task", 0, 0),
    ("((struct worker *)0x0)", "scheduled", 0, 0),
    ("((struct global_cwq *)0x0)", "busy_hash", 0, 0),
    ("((struct global_cwq *)0x0)", "idle_list", 0, 0),
    ("((struct worker *)0x0)", "current_work", 0, 0),
    ("((struct worker *)0x0)", "hentry", 0, 0),
    ("((struct cpu_workqueue_struct *)0x0)", "delayed_works", 0, 0),
    ("((struct workqueue_struct *)0x0)", "cpu_wq", 0, 0),
    ("((struct delayed_work *)0x0)", "timer", 0, 0),
    ("((struct global_cwq *)0x0)", "worklist", 0, 0),
]

workqueue_offsets_3_7 = [
    ("((struct worker *)0x0)", "entry", 0, 0),
    ("((struct worker *)0x0)", "task", 0, 0),
    ("((struct worker *)0x0)", "scheduled", 0, 0),
    ("((struct global_cwq *)0x0)", "busy_hash", 0, 0),
    ("((struct worker *)0x0)", "current_work", 0, 0),
    ("((struct worker *)0x0)", "hentry", 0, 0),
    ("((struct cpu_workqueue_struct *)0x0)", "delayed_works", 0, 0),
    ("((struct workqueue_struct *)0x0)", "cpu_wq", 0, 0),
    ("((struct global_cwq *)0x0)", "pools", 0, 0),
    ("((struct worker_pool *)0x0)", "idle_list", 0, 0),
    ("((struct worker_pool *)0x0)", "worklist", 0, 0),
    ("sizeof(struct worker_pool)","",0, 1),
]

def print_workqueue_state_3_0(ram_dump) :
    ram_dump.setup_offset_table(workqueue_offsets_3_0)
    print_out_str ("======================= WORKQUEUE STATE ============================")
    per_cpu_offset_addr = ram_dump.addr_lookup("__per_cpu_offset")
    global_cwq_sym_addr = ram_dump.addr_lookup("global_cwq")
    system_wq_addr = ram_dump.addr_lookup("system_long_wq")

    idle_list_offset = ram_dump.get_offset_struct("((struct global_cwq *)0x0)", "idle_list")
    worklist_offset = ram_dump.get_offset_struct("((struct global_cwq *)0x0)", "worklist")
    busy_hash_offset = ram_dump.get_offset_struct("((struct global_cwq *)0x0)", "busy_hash")
    scheduled_offset = ram_dump.get_offset_struct("((struct worker *)0x0)", "scheduled")
    worker_task_offset = ram_dump.get_offset_struct("((struct worker *)0x0)", "task")
    worker_entry_offset = ram_dump.get_offset_struct("((struct worker *)0x0)", "entry")
    offset_comm = ram_dump.get_offset_struct("((struct task_struct *)0x0)", "comm")
    work_entry_offset = ram_dump.get_offset_struct("((struct work_struct *)0x0)", "entry")
    work_hentry_offset = ram_dump.get_offset_struct("((struct worker *)0x0)", "hentry")
    work_func_offset = ram_dump.get_offset_struct("((struct work_struct *)0x0)", "func")
    current_work_offset = ram_dump.get_offset_struct("((struct worker *)0x0)", "current_work")
    cpu_wq_offset = ram_dump.get_offset_struct("((struct workqueue_struct *)0x0)", "cpu_wq")
    unbound_gcwq_addr = ram_dump.addr_lookup("unbound_global_cwq")

    if per_cpu_offset_addr is None :
        per_cpu_offset0 = 0
        per_cpu_offset1 = 0
    else :
        per_cpu_offset0 = ram_dump.read_word(per_cpu_offset_addr)
        per_cpu_offset1 = ram_dump.read_word(per_cpu_offset_addr+4)

    global_cwq_cpu0_addr = global_cwq_sym_addr + per_cpu_offset0

    try :
        idle_list_addr0 = ram_dump.read_word(global_cwq_cpu0_addr + idle_list_offset)
        idle_list_addr1 = ram_dump.read_word(unbound_gcwq_addr + idle_list_offset)
        worklist_addr0 = ram_dump.read_word(global_cwq_cpu0_addr + worklist_offset)
        worklist_addr1 = ram_dump.read_word(unbound_gcwq_addr + worklist_offset)
    except TypeError:
        print "print WORKQUEUE State Aborted..!!!!"
        print_out_str ( "print WORKQUEUE State Aborted..!!!!" )
        return

    s = "<"
    for a in range(0,64) :
        s = s + "I"

    busy_hash0 = ram_dump.read_string(global_cwq_cpu0_addr + busy_hash_offset,s)
    busy_hash1 = ram_dump.read_string(unbound_gcwq_addr + busy_hash_offset,s)
    busy_hash = []
    for a in busy_hash0 :
        busy_hash.append(a)

    for a in busy_hash1 :
        busy_hash.append(a)


    for k in range(0,128) :
        next_busy_worker = busy_hash[k]
        if busy_hash[k] != 0 :
            cnt = 0
            while True :
                worker_addr = next_busy_worker - work_hentry_offset
                worker_task_addr = ram_dump.read_word(worker_addr + worker_task_offset)
                if worker_task_addr is None or worker_task_addr == 0 :
                    break
                taskname = ram_dump.read_cstring(worker_task_addr + offset_comm, 16)
                scheduled_addr = ram_dump.read_word(worker_addr + scheduled_offset)
                current_work_addr = ram_dump.read_word(worker_addr + current_work_offset)
                current_work_func = ram_dump.read_word(current_work_addr + work_func_offset)
                wname = ram_dump.unwind_lookup(current_work_func)
                if wname is not None :
                    worker_name, a = wname
                else :
                    worker_name = "Worker at 0x{0:x}".format(current_work_func)
                print_out_str ("BUSY Workqueue worker : {0} current_work: {1}".format(taskname, worker_name))
                if cnt > 200 :
                    break
                cnt += 1
                next_busy_worker = ram_dump.read_word(worker_addr + work_hentry_offset)
                if next_busy_worker == 0 :
                    break

    for i in (0, 1) :
        if i == 0:
            idle_list_addr = idle_list_addr0
        else :
            idle_list_addr = idle_list_addr1
        next_entry = idle_list_addr
        while True :
            worker_addr = next_entry - worker_entry_offset
            worker_task_addr = ram_dump.read_word(next_entry - worker_entry_offset + worker_task_offset)
            if worker_task_addr is None or worker_task_addr == 0 :
                break

            taskname = ram_dump.read_cstring((worker_task_addr + offset_comm), 16)
            scheduled_addr = ram_dump.read_word(worker_addr + scheduled_offset)
            current_work_addr = ram_dump.read_word(worker_addr + current_work_offset)
            next_entry = ram_dump.read_word(next_entry)
            if current_work_addr != 0 :
                current_work_func = ram_dump.read_word(current_work_addr + work_func_offset)
                wname = ram_dump.unwind_lookup(current_work_func)
                if wname is not None :
                    current_work_name, foo = wname
                else :
                    current_work_name = "worker at 0x{0:x}".format(current_work_func)
            else :
                current_work_func = 0
                current_work_name = "(null)"

            if next_entry == idle_list_addr :
                break


            print_out_str ("IDLE Workqueue worker: {0} current_work: {1}".format(taskname, current_work_name))
            if scheduled_addr == (worker_addr + scheduled_offset) :
                continue

            if (next_entry == idle_list_addr) :
                break

    print_out_str ("Pending workqueue info")
    for i in (0, 1) :
        if i == 0 :
            worklist_addr = worklist_addr0
        else :
            worklist_addr = worklist_addr1
        next_work_entry = worklist_addr
        while True :
            work_func_addr = ram_dump.read_word(next_work_entry - work_entry_offset + work_func_offset)
            next_work_temp = ram_dump.read_word(next_work_entry)
            if next_work_temp == next_work_entry :
                print_out_str ("!!! Cycle in workqueue!")
                break
            next_work_entry = next_work_temp

            if ram_dump.virt_to_phys(work_func_addr) != 0:
                wname = ram_dump.unwind_lookup(work_func_addr)
                if wname is not None :
                    work_func_name, foo = wname
                else :
                    work_func_name = "worker at 0x{0:x}".format(work_func_addr)
                if i == 0 :
                    print_out_str ("Pending unbound entry: {0}".format(work_func_name))
                else :
                    print_out_str ("Pending bound entry: {0}".format(work_func_name))
            if next_work_entry == worklist_addr :
                break

def print_workqueue_state_3_7(ram_dump) :
    ram_dump.setup_offset_table(workqueue_offsets_3_7)
    print_out_str ("======================= WORKQUEUE STATE ============================")
    per_cpu_offset_addr = ram_dump.addr_lookup("__per_cpu_offset")
    global_cwq_sym_addr = ram_dump.addr_lookup("global_cwq")

    pools_offset = ram_dump.get_offset_struct("((struct global_cwq *)0x0)", "pools")
    worklist_offset = ram_dump.get_offset_struct("((struct global_cwq *)0x0)", "worklist")
    busy_hash_offset = ram_dump.get_offset_struct("((struct global_cwq *)0x0)", "busy_hash")
    scheduled_offset = ram_dump.get_offset_struct("((struct worker *)0x0)", "scheduled")
    worker_task_offset = ram_dump.get_offset_struct("((struct worker *)0x0)", "task")
    worker_entry_offset = ram_dump.get_offset_struct("((struct worker *)0x0)", "entry")
    offset_comm = ram_dump.get_offset_struct("((struct task_struct *)0x0)", "comm")
    work_entry_offset = ram_dump.get_offset_struct("((struct work_struct *)0x0)", "entry")
    work_hentry_offset = ram_dump.get_offset_struct("((struct worker *)0x0)", "hentry")
    work_func_offset = ram_dump.get_offset_struct("((struct work_struct *)0x0)", "func")
    current_work_offset = ram_dump.get_offset_struct("((struct worker *)0x0)", "current_work")
    cpu_wq_offset = ram_dump.get_offset_struct("((struct workqueue_struct *)0x0)", "cpu_wq")
    pool_idle_offset = ram_dump.get_offset_struct("((struct worker_pool *)0x0)", "idle_list")
    worker_pool_size = ram_dump.get_offset_struct("sizeof(struct worker_pool)","")
    pending_work_offset = ram_dump.get_offset_struct("((struct worker_pool *)0x0)", "worklist")
    unbound_gcwq_addr = ram_dump.addr_lookup("unbound_global_cwq")
    cpu_present_bits_addr = ram_dump.addr_lookup("cpu_present_bits");
    cpu_present_bits = ram_dump.read_word(cpu_present_bits_addr)
    cpus = bin(cpu_present_bits).count('1')


    s = "<"
    for a in range(0,64) :
        s = s + "I"

    for i in range(0,cpus) :
        busy_hash = []
        if per_cpu_offset_addr is None :
            offset = 0
        else :
            offset = ram_dump.read_word(per_cpu_offset_addr + 4*i)
        workqueue_i = global_cwq_sym_addr + offset
        busy_hashi = ram_dump.read_string(workqueue_i + busy_hash_offset,s)
        for a in busy_hashi :
            busy_hash.append(a)

        for k in range(0,64) :
            next_busy_worker = busy_hash[k]
            if busy_hash[k] != 0 :
                cnt = 0

                while True :
                    worker_addr = next_busy_worker - work_hentry_offset
                    worker_task_addr = ram_dump.read_word(worker_addr + worker_task_offset)
                    if worker_task_addr is None or worker_task_addr == 0 :
                        break
                    taskname = ram_dump.read_cstring(worker_task_addr + offset_comm, 16)
                    scheduled_addr = ram_dump.read_word(worker_addr + scheduled_offset)
                    current_work_addr = ram_dump.read_word(worker_addr + current_work_offset)
                    current_work_func = ram_dump.read_word(current_work_addr + work_func_offset)
                    wname = ram_dump.unwind_lookup(current_work_func)
                    if wname is not None :
                        worker_name, a = wname
                    else :
                        worker_name = "Worker at 0x{0:x}".format(current_work_func)
                    print_out_str ("BUSY Workqueue worker : {0} current_work: {1}".format(taskname, worker_name))
                    if cnt > 200 :
                        break
                    cnt += 1
                    next_busy_worker = ram_dump.read_word(worker_addr + work_hentry_offset)
                    if next_busy_worker == 0 :
                        break

        worker_pool = workqueue_i + pools_offset
        # Need better way to ge the number of pools...
        for k in range(0, 2) :
            worker_pool_i = worker_pool + k * worker_pool_size

            idle_list_addr = worker_pool_i + pool_idle_offset
            next_entry = ram_dump.read_word(idle_list_addr)
            while True :
                worker_addr = next_entry - worker_entry_offset
                worker_task_addr = ram_dump.read_word(next_entry - worker_entry_offset + worker_task_offset)
                if worker_task_addr is None or worker_task_addr == 0 :
                    break

                taskname = ram_dump.read_cstring((worker_task_addr + offset_comm), 16)
                scheduled_addr = ram_dump.read_word(worker_addr + scheduled_offset)
                current_work_addr = ram_dump.read_word(worker_addr + current_work_offset)
                next_entry = ram_dump.read_word(next_entry)
                if current_work_addr != 0 :
                    current_work_func = ram_dump.read_word(current_work_addr + work_func_offset)
                    wname = ram_dump.unwind_lookup(current_work_func)
                    if wname is not None :
                        current_work_name, foo = wname
                    else :
                        current_work_name = "worker at 0x{0:x}".format(current_work_func)
                else :
                    current_work_func = 0
                    current_work_name = "(null)"

                if next_entry == idle_list_addr :
                    break


                print_out_str ("IDLE Workqueue worker: {0} current_work: {1}".format(taskname, current_work_name))
                if scheduled_addr == (worker_addr + scheduled_offset) :
                    continue

                if (next_entry == idle_list_addr) :
                    break

            worklist_addr = worker_pool_i + pending_work_offset
            next_work_entry = worklist_addr
            while ram_dump.read_word(next_work_entry) != next_work_entry :
                work_func_addr = ram_dump.read_word(next_work_entry - work_entry_offset + work_func_offset)
                next_work_temp = ram_dump.read_word(next_work_entry)
                if next_work_temp == next_work_entry :
                    print_out_str ("!!! Cycle in workqueue!")
                    break
                next_work_entry = next_work_temp

                if ram_dump.virt_to_phys(work_func_addr) != 0:
                    work_func_name, foo = ram_dump.unwind_lookup(work_func_addr)
                    if i == 0 :
                        print_out_str ("Pending unbound entry: {0}".format(work_func_name))
                    else :
                        print_out_str ("Pending bound entry: {0}".format(work_func_name))
                if next_work_entry == worklist_addr :
                    break

def print_workqueue_state(ram_dump) :
        ver = ram_dump.version
        hw_id = ram_dump.hw_id
        if re.search('3.0.\d',ver) is not None :
            print_workqueue_state_3_0(ram_dump)
        if re.search('3.4.\d',ver) is not None :
            if hw_id == 8226 :
                print_workqueue_state_3_7(ram_dump)
            if hw_id == 8610 :
                print_workqueue_state_3_7(ram_dump)
            if hw_id == 8210 :
                print_workqueue_state_3_7(ram_dump)
            if hw_id == 8026 :
                print_workqueue_state_3_7(ram_dump)
        else :
            print_workqueue_state_3_0(ram_dump)
        if re.search('3.7.\d',ver) is not None :
            print_workqueue_state_3_7(ram_dump)
