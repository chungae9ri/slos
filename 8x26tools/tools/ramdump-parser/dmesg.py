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

import re
import string
from print_out import *

def cleanupString(unclean_str):
    if unclean_str is None :
        return str
    else :
        return ''.join([c for c in unclean_str if c in string.printable])

def extract_dmesg_flat(ramdump) :
    addr = ramdump.addr_lookup("__log_buf")
    size = ramdump.get_offset_struct("sizeof(__log_buf)","")
    dmesg = ramdump.read_physical(ramdump.virt_to_phys(addr), size)
    print_out_str(cleanupString(dmesg.decode('ascii','ignore')))

offsets_binary_log = [
    ("((struct log *)0x0)", "ts_nsec", 0, 0),
    ("((struct log *)0x0)", "len", 0, 0),
    ("((struct log *)0x0)", "text_len", 0, 0),
    ("((struct log *)0x0)", "dict_len", 0, 0),
    ("((struct log *)0x0)", "facility", 0, 0),
    ("((struct log *)0x0)", "flags", 0, 0),
    ("sizeof(struct log)","",0, 1),
]


def log_from_idx(ramdump, idx, logbuf) :
    len_offset = ramdump.get_offset_struct("((struct log *)0x0)", "len")

    msg = logbuf + idx
    msg_len = ramdump.read_word(msg + len_offset)
    if (msg_len == 0) :
        return logbuf
    else :
        return msg

def log_next(ramdump, idx, logbuf) :
    len_offset = ramdump.get_offset_struct("((struct log *)0x0)", "len")
    msg = idx

    msg_len = ramdump.read_halfword(msg + len_offset)
    if (msg_len == 0) :
# LGE_CHANGE_S: workaround for kernel V3.7 log_buf parsing bug
        #msg = logbuf
        #return ramdump.read_halfword(msg + len_offset)
		return -1
# LGE_CHANGE_E
    else :
        return idx + msg_len


def extract_dmesg_binary(ramdump) :
    ramdump.setup_offset_table(offsets_binary_log)
    first_idx_addr = ramdump.addr_lookup("log_first_idx")
    last_idx_addr = ramdump.addr_lookup("log_next_idx")
    logbuf_addr = ramdump.addr_lookup("__log_buf")
    time_offset = ramdump.get_offset_struct("((struct log *)0x0)", "ts_nsec")
    len_offset = ramdump.get_offset_struct("((struct log *)0x0)", "len")
    text_len_offset = ramdump.get_offset_struct("((struct log *)0x0)", "text_len")
    log_size = ramdump.get_offset_struct("sizeof(struct log)","")

    first_idx = ramdump.read_word(first_idx_addr)
    last_idx = ramdump.read_word(last_idx_addr)

    curr_idx = logbuf_addr + first_idx

# LGE_CHANGE_S: workaround for kernel V3.7 log_buf parsing bug
#    while curr_idx != logbuf_addr + last_idx :
    while (curr_idx != logbuf_addr + last_idx) and (curr_idx != -1) :
# LGE_CHANGE_E
        timestamp = ramdump.read_dword(curr_idx + time_offset)
        text_len = ramdump.read_halfword(curr_idx + text_len_offset)
        text_str = ramdump.read_cstring(curr_idx + log_size, text_len)
        for partial in text_str.split('\n') :
            f = "[{0:>5}.{1:0>6d}] {2}".format(timestamp/1000000000, (timestamp % 1000000000) / 1000, partial)
            print_out_str(f)
        curr_idx = log_next(ramdump, curr_idx, logbuf_addr)

def extract_dmesg(ramdump) :
   if re.search('3.7.\d',ramdump.version) is not None :
        extract_dmesg_binary(ramdump)
   else :
        extract_dmesg_flat(ramdump)
