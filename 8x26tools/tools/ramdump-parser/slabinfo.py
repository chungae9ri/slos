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

def get_free_pointer(ramdump, s, obj) :
    # just like validate_slab_slab!
    slab_offset_offset = ramdump.get_offset_struct("((struct kmem_cache *)0x0)", "offset")
    slab_offset = ramdump.read_word(s + slab_offset_offset)
    return ramdump.read_word(obj + slab_offset)

def slab_index(ramdump, p, addr, slab) :
    slab_size_offset = ramdump.get_offset_struct("((struct kmem_cache *)0x0)", "size")
    slab_size = ramdump.read_word(slab + slab_size_offset)
    if slab_size is None :
        return -1
    return (p - addr) / slab_size

def get_map(ramdump, slab, page, bitarray) :
    freelist_offset = ramdump.get_offset_struct("((struct page *)0x0)", "freelist")
    freelist = ramdump.read_word(page + freelist_offset)
    p = freelist
    addr = page_address(ramdump, page)
    if addr is None :
        return
    while p != 0 and p is not None:
        idx = slab_index(ramdump, p, addr, slab)
        if idx >= len(bitarray)  or idx < 0:
            return
        bitarray[idx] = 1
        p = get_free_pointer(ramdump, slab, p)

def get_track(ramdump, slab, obj, track_type) :
    track_size = ramdump.get_offset_struct("sizeof(struct track)","")
    slab_offset_offset = ramdump.get_offset_struct("((struct kmem_cache *)0x0)", "offset")
    slab_inuse_offset = ramdump.get_offset_struct("((struct kmem_cache *)0x0)", "inuse")
    slab_offset = ramdump.read_word(slab + slab_offset_offset)
    slab_inuse = ramdump.read_word(slab + slab_inuse_offset)
    if slab_offset != 0 :
        p = obj + slab_offset + 4
    else :
        p = obj + slab_inuse
    return p + track_type*track_size

def print_track(ramdump, slab, obj, track_type, out_file) :
    p = get_track(ramdump, slab, obj, track_type)
    track_addrs_offset = ramdump.get_offset_struct("((struct track *)0x0)","addrs")
    start = p + track_addrs_offset
    if track_type == 0 :
        out_file.write("   ALLOC STACK\n")
    else :
        out_file.write("   FREE STACK\n")
    for i in range(0, 16) :
        a = ramdump.read_word(start + 4*i)
        if a == 0 :
            break
        look = ramdump.unwind_lookup(a)
        if look is None :
            return
        symname, offset = look
        out_file.write("      [<{0:x}>] {1}+0x{2:x}\n".format(a, symname, offset))
    out_file.write ("\n")

def get_nobjects(ramdump, page) :
    if re.search('3\.0\.\d',ramdump.version) is not None :
        n_objects_offset = ramdump.get_offset_struct("((struct page *)0x0)", "objects")
        n_objects = ramdump.read_halfword(page + n_objects_offset)
        return n_objects
    if re.search('3\.4\.\d',ramdump.version) is not None or re.search('3\.7\.\d',ramdump.version) is not None :
        # The objects field is now a bit field. This confuses GDB as it thinks the
        # offset is always 0. Work around this for now
        map_count_offset = ramdump.get_offset_struct("((struct page *)0x0)", "_mapcount")
        count = ramdump.read_word(page + map_count_offset)
        if count is None :
            return None
        n_objects = (count >> 16) & 0xFFFF
        return n_objects

def print_slab(ramdump, slab_start, slab, page, out_file) :
    p = slab_start
    if page is None :
        return
    n_objects = get_nobjects(ramdump, page)
    if n_objects is None :
        return
    slab_size_offset = ramdump.get_offset_struct("((struct kmem_cache *)0x0)", "size")
    slab_size = ramdump.read_word(slab + slab_size_offset)
    if slab_size is None :
        return
    slab_max_offset = ramdump.get_offset_struct("((struct kmem_cache *)0x0)", "max")
    slab_max = ramdump.read_word(slab + slab_max_offset)
    if slab_max is None :
        return
    bitarray = [0] * slab_max
    addr = page_address(ramdump, page)
    get_map(ramdump, slab, page, bitarray)
    while p < slab_start +  n_objects*slab_size :
        idx = slab_index(ramdump, p, addr, slab)
        bitidx = slab_index(ramdump, p, addr, slab)
        if bitidx >= len(bitarray) or bitidx < 0:
            return
        if bitarray[bitidx] == 1 :
            out_file.write ("   Object {0:x}-{1:x} FREE\n".format(p, p+slab_size))
        else :
            out_file.write ("   Object {0:x}-{1:x} ALLOCATED\n".format(p, p+slab_size))
        if ramdump.is_config_defined("CONFIG_SLUB_DEBUG_ON") :
            print_track(ramdump, slab, p, 0, out_file)
            print_track(ramdump, slab, p, 1, out_file)
        p = p + slab_size

def print_slab_page_info(ramdump, slab, slab_node, start, out_file) :
    page = ramdump.read_word(start)
    if page == 0 :
        return
    slab_lru_offset = ramdump.get_offset_struct("((struct page *)0x0)", "lru")
    page_flags_offset = ramdump.get_offset_struct("((struct page *)0x0)", "flags")
    slab_node_offset = ramdump.get_offset_struct("((struct kmem_cache *)0x0)", "size")
    while page != start:
        if page is None :
            return
        page = page - slab_lru_offset
        page_flags = ramdump.read_word(page + page_flags_offset)
        page_addr = page_address(ramdump, page)
        print_slab(ramdump, page_addr, slab, page, out_file)
        page = ramdump.read_word(page + slab_lru_offset)

def print_per_cpu_slab_info(ramdump, slab, slab_node, start, out_file) :
    page = ramdump.read_word(start)
    if page == 0 :
        return
    page_flags_offset = ramdump.get_offset_struct("((struct page *)0x0)", "flags")
    if page is None :
        return
    page_flags = ramdump.read_word(page + page_flags_offset)
    page_addr = page_address(ramdump, page)
    print_slab(ramdump, page_addr, slab, page, out_file)

slab_offsets_common = [
        ("((struct kmem_cache *)0x0)", "list", 0, 0),
        ("((struct kmem_cache *)0x0)", "name", 0, 0),
        ("((struct kmem_cache *)0x0)", "node", 0, 0),
        ("((struct kmem_cache *)0x0)", "size", 0, 0),
        ("((struct kmem_cache *)0x0)", "offset", 0, 0),
        ("((struct kmem_cache *)0x0)", "max", 0, 0),
        ("((struct kmem_cache *)0x0)", "inuse", 0, 0),
        ("((struct kmem_cache *)0x0)", "cpu_slab", 0, 0),
        ("((struct kmem_cache_cpu *)0x0)", "page", 0, 0),
        ("((struct kmem_cache_node *)0x0)", "partial", 0, 0),
        ("((struct kmem_cache_node *)0x0)", "full", 0, 0),
        ("((struct page *)0x0)", "lru", 0, 0),
        ("((struct page *)0x0)", "objects", 0, 0),
        ("((struct page *)0x0)", "freelist", 0, 0),
        ("((struct track *)0x0)","addrs",0,0),
        ("sizeof(struct track)","",0, 1),
]

slab_offsets_3_4 = [
        ("((struct page *)0x0)", "_mapcount", 0, 0),
]

# based on validate_slab_cache. Currently assuming there is only one numa node
# in the system because the code to do that correctly is a big pain. This will
# need to be changed if we ever do NUMA properly.
def print_slab_info(ramdump) :
    if re.search('3\.0\.\d',ramdump.version) is not None :
        ramdump.setup_offset_table(slab_offsets_common)
    if re.search('3\.4\.\d',ramdump.version) is not None or re.search('3\.7\.\d', ramdump.version) is not None :
        ramdump.setup_offset_table(slab_offsets_common + slab_offsets_3_4)
    slab_out = open("{0}/slabs.txt".format(ramdump.outdir),"wb")
    original_slab = ramdump.addr_lookup("slab_caches")
    per_cpu_offset = ramdump.addr_lookup("__per_cpu_offset")
    cpu_present_bits_addr = ramdump.addr_lookup("cpu_present_bits");
    cpu_present_bits = ramdump.read_word(cpu_present_bits_addr)
    cpus = bin(cpu_present_bits).count('1')
    slab_list_offset = ramdump.get_offset_struct("((struct kmem_cache *)0x0)", "list")
    slab_name_offset = ramdump.get_offset_struct("((struct kmem_cache *)0x0)", "name")
    slab_node_offset = ramdump.get_offset_struct("((struct kmem_cache *)0x0)", "node")
    cpu_cache_page_offset = ramdump.get_offset_struct("((struct kmem_cache_cpu *)0x0)", "page")
    cpu_slab_offset = ramdump.get_offset_struct("((struct kmem_cache *)0x0)", "cpu_slab")
    slab_partial_offset = ramdump.get_offset_struct("((struct kmem_cache_node *)0x0)", "partial")
    slab_full_offset = ramdump.get_offset_struct("((struct kmem_cache_node *)0x0)", "full")
    slab = ramdump.read_word(original_slab)
    while slab != original_slab :
        slab = slab - slab_list_offset
        slab_name_addr = ramdump.read_word(slab + slab_name_offset)
        # actually an array but again, no numa
        slab_node_addr = ramdump.read_word(slab + slab_node_offset)
        slab_node = ramdump.read_word(slab_node_addr)
        slab_name = ramdump.read_cstring(slab_name_addr, 48)
        cpu_slab_addr = ramdump.read_word(slab + cpu_slab_offset)
        print_out_str ("Parsing slab {0}".format(slab_name))
        slab_out.write ("{0:x} slab {1} {2:x}\n".format(slab, slab_name, slab_node_addr))
        print_slab_page_info(ramdump, slab, slab_node, slab_node_addr + slab_partial_offset, slab_out)
        print_slab_page_info(ramdump, slab, slab_node, slab_node_addr + slab_full_offset, slab_out)

        for i in range(0,cpus) :
            cpu_slabn_addr = cpu_slab_addr + ramdump.read_word(per_cpu_offset + 4*i)
            print_per_cpu_slab_info(ramdump, slab, slab_node, cpu_slabn_addr + cpu_cache_page_offset, slab_out)

        slab = ramdump.read_word(slab + slab_list_offset)
    print_out_str("---wrote slab information to {0}/slabs.txt".format(ramdump.outdir))
