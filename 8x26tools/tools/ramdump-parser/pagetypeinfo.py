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
from mm import *

pagetype_offsets  = [
        ("((struct pglist_data *)0x0)", "node_zones", 0, 0),
        ("sizeof(struct zone)","",0, 1),
        ("sizeof(struct free_area)","",0, 1),
        ("((struct zone *)0x0)","name",0, 0),
        ("((struct zone *)0x0)","free_area",0, 0),
        ("((struct zone *)0x0)","present_pages",0, 0),
        ("((struct free_area *)0x0)","free_list",0, 0),
        ("((struct free_area *)0x0)","nr_free",0, 0),
]

def print_pagetype_info_per_zone(ramdump, zone, migrate_types) :

    free_area_offset = ramdump.get_offset_struct("((struct zone *)0x0)","free_area")
    free_area_size = ramdump.get_offset_struct("sizeof(struct free_area)","")
    free_list_offset = ramdump.get_offset_struct("((struct free_area *)0x0)","free_list")
    migratetype_names = ramdump.addr_lookup("migratetype_names")
    zone_name_offset = ramdump.get_offset_struct("((struct zone *)0x0)","name")
    zname_addr = ramdump.read_word(zone + zone_name_offset)
    zname = ramdump.read_cstring(zname_addr, 12)
    is_corrupt = False
    total_bytes = 0

    for mtype in range(0, migrate_types) :
        mname_addr = ramdump.read_word(migratetype_names + mtype * 4)
        mname = ramdump.read_cstring(mname_addr, 12)
        pageinfo = ("zone {0:8} type {1:12} ".format(zname, mname))
        nums = ""
        total_type_bytes = 0
        for order in range(0, 11) :

            area = zone + free_area_offset + order * free_area_size

            orig_free_list = area + free_list_offset + 8 * mtype
            curr = orig_free_list
            pg_count = -1
            first = True
            while True :
                pg_count = pg_count + 1
                next_p = ramdump.read_word(curr)
                if next_p == curr :
                    if not first :
                       is_corrupt = True
                    break
                first = False
                curr = next_p
                if curr == orig_free_list :
                    break
            nums = nums+("{0:6}".format(pg_count))
            total_type_bytes = total_type_bytes + pg_count * 4096 * (2**order)
        print_out_str( pageinfo + nums + " = {0} MB".format(total_type_bytes/(1024*1024)))
        total_bytes = total_bytes + total_type_bytes

    print_out_str("Approximate total for zone {0}: {1} MB\n".format(zname, total_bytes/(1024*1024)))
    if is_corrupt :
        print_out_str ("!!! Numbers may not be accurate due to list corruption!")

def print_pagetypeinfo(ramdump) :
    ramdump.setup_offset_table(pagetype_offsets)
    gdb_cmd = NamedTemporaryFile(mode='w+t', delete=False)
    gdb_out = NamedTemporaryFile(mode='w+t', delete=False)
    gdb_cmd.write("print /d MIGRATE_TYPES\n")
    gdb_cmd.write("print /d __MAX_NR_ZONES\n")
    gdb_cmd.flush()
    gdb_cmd.close()
    gdb_out.close()
    stream = os.system("{0} -x {1} --batch {2} > {3}".format(ramdump.gdb_path, gdb_cmd.name, ramdump.vmlinux, gdb_out.name))
    a = open(gdb_out.name)
    results = a.readlines()
    vals = []
    for r in results :
        s = r.split(' ')
        vals.append(int(s[2].rstrip(),10))
    a.close()
    os.remove(gdb_out.name)
    os.remove(gdb_cmd.name)
    migrate_types = vals[0]
    max_nr_zones = vals[1]

    contig_page_data = ramdump.addr_lookup("contig_page_data")
    node_zones_offset = ramdump.get_offset_struct("((struct pglist_data *)0x0)", "node_zones")
    present_pages_offset = ramdump.get_offset_struct("((struct zone *)0x0)", "present_pages")
    sizeofzone = ramdump.get_offset_struct("sizeof(struct zone)","")
    zone = contig_page_data + node_zones_offset

    while zone < (contig_page_data + node_zones_offset + max_nr_zones * sizeofzone)  :
         present_pages = ramdump.read_word(zone + present_pages_offset)
         if not not present_pages :
             print_pagetype_info_per_zone(ramdump, zone, migrate_types)

         zone = zone + sizeofzone


