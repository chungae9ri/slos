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

irq_offsets = [
    ("((struct irq_chip *)0x0)", "name", 0, 0),
    ("((struct irq_desc *)0x0)", "kstat_irqs", 0, 0),
    ("((struct irqaction *)0x0)", "name", 0,0),
    ("((struct irq_desc *)0x0)", "action", 0, 0),
    ("((struct irq_desc *)0x0)", "irq_data", 0, 0),
    ("((struct irq_desc *)0x0)", "irq_count", 0, 0),
    ("((struct irq_desc *)0x0)", "handle_irq", 0, 0),
    ("((struct irq_data *)0x0)", "chip", 0, 0),
    ("((struct irq_data *)0x0)", "irq", 0, 0),
    ("sizeof(irq_desc[0])", "", 0, 1),
]

def print_irq_state_3_0(ram_dump) :
    ram_dump.setup_offset_table(irq_offsets)
    print_out_str ("=========================== IRQ STATE ===============================")
    per_cpu_offset_addr = ram_dump.addr_lookup("__per_cpu_offset")
    cpu_present_bits_addr = ram_dump.addr_lookup("cpu_present_bits");
    cpu_present_bits = ram_dump.read_word(cpu_present_bits_addr)
    cpus = bin(cpu_present_bits).count('1')
    irq_desc = ram_dump.addr_lookup("irq_desc")
    foo, irq_desc_size = ram_dump.unwind_lookup(irq_desc, 1)
    h_irq_offset = ram_dump.get_offset_struct("((struct irq_desc *)0x0)", "handle_irq")
    irq_num_offset = ram_dump.get_offset_struct("((struct irq_data *)0x0)", "irq")
    irq_data_offset = ram_dump.get_offset_struct("((struct irq_desc *)0x0)", "irq_data")
    irq_count_offset = ram_dump.get_offset_struct("((struct irq_desc *)0x0)", "irq_count")
    irq_chip_offset = ram_dump.get_offset_struct("((struct irq_data *)0x0)", "chip")
    irq_action_offset = ram_dump.get_offset_struct("((struct irq_desc *)0x0)", "action")
    action_name_offset = ram_dump.get_offset_struct("((struct irqaction *)0x0)", "name")
    kstat_irqs_offset = ram_dump.get_offset_struct("((struct irq_desc *)0x0)", "kstat_irqs")
    chip_name_offset = ram_dump.get_offset_struct("((struct irq_chip *)0x0)", "name")
    irq_desc_entry_size = ram_dump.get_offset_struct("sizeof(irq_desc[0])", "")
    cpu_str = ""

    for i in range(0,cpus) :
        cpu_str = cpu_str + "{0:10} ".format("CPU{0}".format(i))

    print_out_str ("{0:4} {1} {2:30} {3:10}".format("IRQ", cpu_str, "Name", "Chip"))
    for i in range(0, irq_desc_size, irq_desc_entry_size) :
        irqnum = ram_dump.read_word(irq_desc+i+irq_num_offset)
        irqcount = ram_dump.read_word(irq_desc+i+irq_count_offset)
        action = ram_dump.read_word(irq_desc+i+irq_action_offset)
        kstat_irqs_addr = ram_dump.read_word(irq_desc+i+kstat_irqs_offset)
        irq_stats_str = ""

        for j in range(0, cpus) :
            if per_cpu_offset_addr is None :
                offset = 0
            else :
                offset = ram_dump.read_word(per_cpu_offset_addr + 4*j)
            irq_statsn = ram_dump.read_word(kstat_irqs_addr + offset)
            irq_stats_str = irq_stats_str + "{0:10} ".format("{0}".format(irq_statsn))

        chip = ram_dump.read_word(irq_desc+i+irq_data_offset+irq_chip_offset)
        chip_name_addr = ram_dump.read_word(chip+chip_name_offset)
        chip_name = ram_dump.read_cstring(chip_name_addr, 48)

        if action != 0 :
            name_addr = ram_dump.read_word(action+action_name_offset)
            name = ram_dump.read_cstring(name_addr, 48)
            print_out_str ("{0:4} {1} {2:30} {3:10}".format(irqnum, irq_stats_str, name, chip_name))


def print_irq_state(ram_dump) :
    irq_desc = ram_dump.addr_lookup("irq_desc")
    if ram_dump.is_config_defined("CONFIG_SPARSE_IRQ") or irq_desc is None :
        print_out_str ("!!! IRQ dumping on targets with sparse IRQs is not supported currently")
        print_out_str ("!!! This is on a TODO list somewhere to be fixed")
        print_out_str ("!!! This is not considered an error with the parser!")
        return

    ver = ram_dump.version
    if re.search('3.0.\d',ver) is not None :
        print_irq_state_3_0(ram_dump)
    if re.search('3.4.\d',ver) is not None :
        print_irq_state_3_0(ram_dump)
