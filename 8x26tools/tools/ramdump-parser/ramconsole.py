# Copyright (c) 2013, LG Electronics. All rights reserved.


import sys
import re
import os
import struct
import datetime
import array
import string
import bisect
import traceback
import time
from subprocess import *
from optparse import OptionParser
from optparse import OptionGroup
from struct import unpack
from ctypes import *
from tempfile import *
from print_out import *

persistent_ram_zone_offsets = [
    ("((struct persistent_ram_zone *)0x0)", "old_log", 0, 0),
    ("((struct persistent_ram_zone *)0x0)", "old_log_size", 0, 0),
]

def print_last_kmsg(ram_dump) :
    ram_dump.setup_offset_table(persistent_ram_zone_offsets)
    old_log_offset = ram_dump.get_offset_struct("((struct persistent_ram_zone *)0x0)", "old_log")
    old_log_size_offset = ram_dump.get_offset_struct("((struct persistent_ram_zone *)0x0)", "old_log_size")
    ram_console_zone_addr = ram_dump.read_word(ram_dump.addr_lookup("ram_console_zone"))

    old_log = ram_dump.read_word(ram_console_zone_addr + old_log_offset)
    old_log_size = ram_dump.read_word(ram_console_zone_addr + old_log_size_offset)

    if old_log != 0 :
        buf = ram_dump.read_physical(ram_dump.virt_to_phys(old_log), old_log_size)

        try :
            for f in buf.split('\n') :
                print_out_str(f)
        except :
            print_out_str ("extraction error was occured!! at print_last_kmsg")

    else :

        print_out_str ("old_log is null. there is no last_kmsg!!")

def print_ramconsole(ram_dump) :
    if ram_dump.tz_start == 0xFE800000 :
    	prz_buffer = ram_dump.read_word(0xFE805024, False)
    else :
    	prz_buffer = ram_dump.read_word(0xFC42B024, False)

    if ram_dump.read_word(prz_buffer, False) != 0x43474244 :
        print_out_str("ram console signature is not correct")
        return

    start = ram_dump.read_word(prz_buffer + 0x4, False)
    size = ram_dump.read_word(prz_buffer + 0x8, False)

    if size > start :
        buf1 = ram_dump.read_physical(prz_buffer + 0xC + start, size - start)
        try :
            for f in buf1.split('\n') :
                print_out_str(f)
        except :
            print_out_str ("extraction error was occured!!")

    if start > 0 :
        buf2 = ram_dump.read_physical(prz_buffer + 0xC, start)
        try :
            for f in buf2.split('\n') :
                print_out_str(f)
        except :
            print_out_str ("extraction error was occured!! at print_ramconsole")

def print_crash_log(ram_dump) :
    if ram_dump.tz_start == 0xFE800000 :
        prz_buffer = ram_dump.read_word(0xFE805024, False)
        size = ram_dump.read_word(0xFE805028, False)
    else :
        prz_buffer = ram_dump.read_word(0xFC42B024, False)
        size = ram_dump.read_word(0xFC42B028, False)

    try :
        log_buf_start = ram_dump.read_word(prz_buffer + size, False)
        log_buf_end = ram_dump.read_word(prz_buffer + size + 4, False)
	
        buf = ram_dump.read_physical(log_buf_start, log_buf_end - log_buf_start)        
        for f in buf.split('\n') :
            print_out_str(f)
    except :
        print_out_str ("extraction error was occured!! at print_crash_log")
