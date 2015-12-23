# Copyright (c) 2013, The Linux Foundation. All rights reserved.
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

gpuinfo_offsets = [
        ("((struct adreno_device *)0x0)", "dev", 0, 0),
        ("((struct adreno_device *)0x0)", "ringbuffer", 0, 0),
        ("((struct adreno_ringbuffer *)0x0)", "sizedwords", 0, 0),
        ("((struct adreno_ringbuffer *)0x0)", "wptr", 0, 0),
        ("((struct adreno_ringbuffer *)0x0)", "buffer_desc", 0, 0),
        ("((struct adreno_ringbuffer *)0x0)", "memptrs_desc", 0, 0),
        ("((struct kgsl_memdesc *)0x0)", "physaddr", 0, 0),
        ("((struct kgsl_device *)0x0)", "memstore", 0, 0),
        ("((struct kgsl_device *)0x0)", "snapshot", 0, 0),
        ("((struct kgsl_device *)0x0)", "snapshot_size", 0, 0),
        ("((struct kgsl_device *)0x0)", "snapshot_timestamp", 0, 0),
]

def extract_gpuinfo(ramdump) :
        ramdump.setup_offset_table(gpuinfo_offsets)
        adreno_dev_addr = ramdump.addr_lookup("device_3d0")
        kgsl_dev_addr = adreno_dev_addr + ramdump.get_offset_struct(
                                "((struct adreno_device *)0x0)", "dev")
        snapshot = ramdump.read_word(kgsl_dev_addr + ramdump.get_offset_struct(
                                "((struct kgsl_device *)0x0)", "snapshot"))
        snapshot_size = ramdump.read_word(kgsl_dev_addr +
                        ramdump.get_offset_struct(
                        "((struct kgsl_device *)0x0)", "snapshot_size"))
        snapshot_timestamp = ramdump.read_word(kgsl_dev_addr +
                                ramdump.get_offset_struct(
                                "((struct kgsl_device *)0x0)",
                                "snapshot_timestamp"))
        ringbuffer_offset = ramdump.get_offset_struct(
                                "((struct adreno_device *)0x0)", "ringbuffer")
        ringbuffer_addr = ramdump.read_word(adreno_dev_addr +
                ringbuffer_offset +
                ramdump.get_offset_struct(
                        "((struct adreno_ringbuffer *)0x0)", "buffer_desc") +
                ramdump.get_offset_struct(
                        "((struct kgsl_memdesc *)0x0)", "physaddr"))
        memptrs_addr = ramdump.read_word(adreno_dev_addr + ringbuffer_offset +
                        ramdump.get_offset_struct(
                        "((struct adreno_ringbuffer *)0x0)", "memptrs_desc") +
                        ramdump.get_offset_struct(
                        "((struct kgsl_memdesc *)0x0)", "physaddr"))
        memstore_addr = ramdump.read_word(kgsl_dev_addr +
                        ramdump.get_offset_struct(
                        "((struct kgsl_device *)0x0)", "memstore") +
                        ramdump.get_offset_struct(
                        "((struct kgsl_memdesc *)0x0)", "physaddr"))
        ringbuffer_size = ramdump.read_word(adreno_dev_addr +
                          ringbuffer_offset +
                          ramdump.get_offset_struct(
                          "((struct adreno_ringbuffer *)0x0)", "sizedwords"))
        print_out_str ("Ringbuffer address: {0:x}, Ringbuffer sizedwords: "
                        "{1:x}, Memstore address: {2:x}, Memptrs address: "
                        "{3:x}".format(ringbuffer_addr, ringbuffer_size,
                        memstore_addr, memptrs_addr))
        print_out_str ("Sanpshot addr: {0:x}, Snapshot size: {1:x}, "
                        "Snapshot timestamp:{2:x}".format(snapshot,
                        snapshot_size, snapshot_timestamp))
        current_context = ramdump.read_word(int(memstore_addr) + 32, False)
        retired_timestamp = ramdump.read_word(int(memstore_addr) + 8, False)
        i = 0
        for i in range(0, int(ringbuffer_size)) :
                data = ramdump.read_word(int(ringbuffer_addr) + (i * 4), False)
                if int(data) == int(retired_timestamp) :
                        break
        i = i * 4
        print_out_str ("Current context: {0:x}, Global eoptimestamp: {1:x} "
                        "found at Ringbuffer[{2:x}]".format(current_context,
                        retired_timestamp, i))
