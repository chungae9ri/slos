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

# assuming cache way size of 8, fix this for badger probably
cache_way = 8

cache_dump_offsets = [
    ("((struct l2_cache_dump *)0x0)","magic_number",0,0),
    ("((struct l2_cache_dump *)0x0)","version",0,0),
    ("((struct l2_cache_dump *)0x0)","line_size",0,0),
    ("((struct l2_cache_dump *)0x0)","total_lines",0,0),
    ("((struct l2_cache_dump *)0x0)","cache",0,0),
    ("((struct l2_cache_line_dump *)0x0)","l2dcrtr0_val",0,0),
    ("((struct l2_cache_line_dump *)0x0)","l2dcrtr1_val",0,0),
    ("((struct l2_cache_line_dump *)0x0)","cache_line_data",0,0),
    ("sizeof(struct l2_cache_line_dump)","",0, 1),
]

def save_l1_dump(ram_dump, out_path, cache_base, size) :
    with open(out_path+"/l1_cache_dump.bin","wb") as cache_file :

        for i in range (0, size) :
            val = ram_dump.read_byte(cache_base + i, False)
            cache_file.write(struct.pack("<B",val))
        print_out_str ("--- Wrote cache dump to {0}/l1_cache_dump.bin".format(out_path))

def parse_cache_dump(ram_dump, out_path, cache_base) :

    ram_dump.setup_offset_table(cache_dump_offsets)

    magic_num_offset = ram_dump.get_offset_struct("((struct l2_cache_dump *)0x0)","magic_number")
    version_offset = ram_dump.get_offset_struct("((struct l2_cache_dump *)0x0)","version")
    line_size_offset = ram_dump.get_offset_struct("((struct l2_cache_dump *)0x0)","line_size")
    total_lines_offset = ram_dump.get_offset_struct("((struct l2_cache_dump *)0x0)","total_lines")
    cache_offset_struct = ram_dump.get_offset_struct("((struct l2_cache_dump *)0x0)","cache")
    l2dcrtr0_offset_struct = ram_dump.get_offset_struct("((struct l2_cache_line_dump *)0x0)","l2dcrtr0_val")
    l2dcrtr1_offset_struct = ram_dump.get_offset_struct("((struct l2_cache_line_dump *)0x0)","l2dcrtr1_val")
    cache_line_data_offset_struct = ram_dump.get_offset_struct("((struct l2_cache_line_dump *)0x0)","cache_line_data")
    cache_line_struct_size = ram_dump.get_offset_struct("sizeof(l2_cache_line_dump)","")

    magic = ram_dump.read_word(cache_base + magic_num_offset, False)
    version = ram_dump.read_word(cache_base + version_offset, False)
    line_size = ram_dump.read_word(cache_base + line_size_offset, False)
    total_lines = ram_dump.read_word(cache_base + total_lines_offset, False)
    cache = ram_dump.read_word(cache_base + cache_offset_struct, False)

    cache_file = open(out_path+"/l2_cache_dump.txt","wb")

    cache_file.write("Magic = {0:x}\n".format(magic))
    cache_file.write("version = {0:x}\n".format(version))
    cache_file.write("line size = {0:x}\n".format(line_size))


    select=0
    lines = total_lines / cache_way

    header_str = "({0:4},{1:1}) {2:5} {3:8} ".format("Set","Way","valid","Address")
    # currently assumes 32 bit word like everything else...
    for i in range(0, 32) :
        header_str=header_str + "{0:8} ".format("Word{0}".format(i))

    header_str=header_str+"{0:8} {1:8}\n".format("L2DCRTR0","L2DCRTR0")


    cache_ptr = cache_base + cache_offset_struct

    for i in range(0,lines) :

        cache_file.write(header_str)

        for j in range(0,cache_way) :
            cache_line_ptr = cache_ptr + (i * cache_way + j)*line_size

            l2dcrtr0_val = ram_dump.read_word(cache_line_ptr + l2dcrtr0_offset_struct, False)
            l2dcrtr1_val = ram_dump.read_word(cache_line_ptr + l2dcrtr1_offset_struct, False)

            # this is valid for krait, will probably need to be more generic

            addr = l2dcrtr1_val & 0xFFFE0000
            addr = addr | (select & 0x0001ff80)
            valid = (l2dcrtr0_val >> 14) & 0x3

            out_str = "({0:4},{1:1}) {2:5} {3:8x} ".format(i,j, valid, addr)

            cache_line_data_ptr = cache_line_ptr + cache_line_data_offset_struct

            for k in range(0, 32) :
                out_str = out_str + "{0:0=8x} ".format(ram_dump.read_word(cache_line_data_ptr + 4*k, False))

            out_str = out_str + "{0:0=8x} {1:0=8x}\n".format(l2dcrtr0_val, l2dcrtr1_val)

            cache_file.write(out_str)
            select = select + 0x10

    cache_file.close()
    print_out_str ("--- Wrote cache dump to {0}/l2_cache_dump.txt".format(out_path))

def print_cache_dump(ram_dump) :
    out_path = ram_dump.outpath
    if not ram_dump.is_config_defined("CONFIG_MSM_CACHE_DUMP") :
        print_out_str ("!!! Cache dumping was not enabled. No cache will be dumped")
        return

    cache_base_addr = ram_dump.addr_lookup("l2_dump")
    cache_base = ram_dump.read_word(cache_base_addr)

    parse_cache_dump(ram_dump, out_path, cache_base)
