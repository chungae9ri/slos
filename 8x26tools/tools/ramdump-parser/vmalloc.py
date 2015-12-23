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

vmalloc_offsets = [
    ("((struct vm_struct *)0x0)", "next", 0, 0),
    ("((struct vm_struct *)0x0)", "addr", 0, 0),
    ("((struct vm_struct *)0x0)", "size", 0, 0),
    ("((struct vm_struct *)0x0)", "flags", 0, 0),
    ("((struct vm_struct *)0x0)", "pages", 0, 0),
    ("((struct vm_struct *)0x0)", "nr_pages", 0, 0),
    ("((struct vm_struct *)0x0)", "phys_addr", 0, 0),
    ("((struct vm_struct *)0x0)", "caller", 0, 0),
]

VM_IOREMAP=0x00000001
VM_ALLOC=0x00000002
VM_MAP=0x00000004
VM_USERMAP=0x00000008
VM_VPAGES=0x00000010
VM_UNLIST=0x00000020

def print_vmalloc_info_3_4(ram_dump, out_path) :
    ram_dump.setup_offset_table(vmalloc_offsets)

    vmlist_addr = ram_dump.addr_lookup("vmlist")
    vmlist = ram_dump.read_word(vmlist_addr)

    vmalloc_out = open("{0}/vmalloc.txt".format(out_path),"wb")

    next_offset = ram_dump.get_offset_struct("((struct vm_struct *)0x0)", "next")
    addr_offset = ram_dump.get_offset_struct("((struct vm_struct *)0x0)", "addr")
    size_offset = ram_dump.get_offset_struct("((struct vm_struct *)0x0)", "size")
    flags_offset = ram_dump.get_offset_struct("((struct vm_struct *)0x0)", "flags")
    pages_offset = ram_dump.get_offset_struct("((struct vm_struct *)0x0)", "pages")
    nr_pages_offset = ram_dump.get_offset_struct("((struct vm_struct *)0x0)", "nr_pages")
    phys_addr_offset = ram_dump.get_offset_struct("((struct vm_struct *)0x0)", "phys_addr")
    caller_offset = ram_dump.get_offset_struct("((struct vm_struct *)0x0)", "caller")

    while (vmlist is not None) and (vmlist != 0) :
        addr = ram_dump.read_word(vmlist + addr_offset)
        caller = ram_dump.read_word(vmlist + caller_offset)
        nr_pages = ram_dump.read_word(vmlist + nr_pages_offset)
        phys_addr = ram_dump.read_word(vmlist + phys_addr_offset)
        flags = ram_dump.read_word(vmlist + flags_offset)
        size = ram_dump.read_word(vmlist + size_offset)

        vmalloc_str = "{0:x}-{1:x} {2:x}".format(addr, addr + size, size)

        if (caller != 0) :
            a = ram_dump.unwind_lookup(caller)
            if a is not None :
                symname, offset = a
                vmalloc_str = vmalloc_str + " {0}+0x{1:x}".format(symname,offset)

        if (nr_pages != 0) :
            vmalloc_str = vmalloc_str + " pages={0}".format(nr_pages)

        if (phys_addr != 0) :
            vmalloc_str = vmalloc_str + " phys={0:x}".format(phys_addr)

        if (flags & VM_IOREMAP) != 0 :
            vmalloc_str = vmalloc_str + " ioremap"

        if (flags & VM_ALLOC) != 0 :
            vmalloc_str = vmalloc_str + " vmalloc"

        if (flags & VM_MAP) != 0 :
            vmalloc_str = vmalloc_str + " vmap"

        if (flags & VM_USERMAP) != 0 :
            vmalloc_str = vmalloc_str + " user"

        if (flags & VM_VPAGES) != 0 :
            vmalloc_str = vmalloc_str + " vpages"

        vmalloc_str = vmalloc_str + "\n"
        vmalloc_out.write(vmalloc_str)

        vmlist = ram_dump.read_word(vmlist + next_offset)

    print_out_str("---wrote vmalloc to {0}/vmalloc.txt".format(out_path))
    vmalloc_out.close()

def print_vmalloc(ram_dump) :
    out_path = ram_dump.outdir
    ver = ram_dump.version
    if re.search('3.0.\d',ver) is not None :
        print_vmalloc_info_3_4(ram_dump, out_path)
    elif re.search('3.4.\d',ver) is not None :
        print_vmalloc_info_3_4(ram_dump, out_path)
    elif re.search('3.7.\d',ver) is not None :
        print_vmalloc_info_3_4(ram_dump, out_path)
