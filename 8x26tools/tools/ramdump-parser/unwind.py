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
from print_out import *

FP = 11
SP = 13
LR = 14
PC = 15
THREAD_SIZE = 8192

class Stackframe () :
    def __init__(self, fp, sp, lr, pc) :
        self.fp = fp
        self.sp = sp
        self.lr = lr
        self.pc = pc

class UnwindCtrlBlock () :
    def __init__ (self) :
        self.vrs = 16*[0]
        self.insn = 0
        self.entries = -1
        self.byte = -1
        self.index = 0

class Unwinder () :
    def __init__(self, ramdump) :
        start = ramdump.addr_lookup("__start_unwind_idx")
        end = ramdump.addr_lookup("__stop_unwind_idx")
        if (start is None) or (end is None) :
            print_out_str ("!!! Could not lookup unwinding information")
            return None
        # addresses
        self.start_idx = start
        self.stop_idx = end
        self.unwind_table = []
        self.ramdump = ramdump
        i = 0
        for addr in range(start,end,8) :
            (a,b) = ramdump.read_string(addr,"<II")
            self.unwind_table.append((a,b,start+8*i))
            i+=1

        ver = ramdump.version
        if re.search('3.0.\d',ver) is not None :
            self.search_idx = self.search_idx_3_0
        else :
            self.search_idx = self.search_idx_3_4
            # index into the table
            self.origin = self.unwind_find_origin()

    def unwind_find_origin(self) :
        start = 0
        stop = len(self.unwind_table)
        while (start < stop) :
            mid = start + ((stop - start) >> 1)
            if (self.unwind_table[mid][0] >= 0x40000000) :
                start = mid + 1
            else :
                stop = mid
        return stop

    def unwind_frame_generic(self, frame) :
        high = 0
        fp = frame.fp

        low = frame.sp
        mask = (THREAD_SIZE) - 1

        high = (low + mask) & (~mask) #ALIGN(low, THREAD_SIZE)

        # /* check current frame pointer is within bounds */
        if (fp < (low + 12) or fp + 4 >= high) :
            return -1

        fp_is_at = self.ramdump.read_word(frame.fp-12)
        sp_is_at = self.ramdump.read_word(frame.fp-8)
        pc_is_at = self.ramdump.read_word(frame.fp-4)

        frame.fp = fp_is_at
        frame.sp = sp_is_at
        frame.pc = pc_is_at

        return 0

    def walk_stackframe_generic(self, frame) :
        while True :
            symname = self.ramdump.addr_to_symbol(frame.pc)
            print_out_str (symname)

            ret = self.unwind_frame_generic(frame)
            if ret < 0 :
                break

    def unwind_backtrace_generic(self, sp, fp, pc) :
        frame = Stackframe()
        frame.fp = fp
        frame.pc = pc
        frame.sp = sp
        walk_stackframe_generic(frame)

    def search_idx_3_4(self, addr) :
        start = 0
        stop = len(self.unwind_table)
        orig = addr

        if (addr < self.start_idx) :
            stop = self.origin
        else :
            start = self.origin

        addr = (addr - self.unwind_table[start][2]) & 0x7fffffff

        while (start < (stop - 1)) :
            mid = start + ((stop - start) >> 1)

            dif =  (self.unwind_table[mid][2] - self.unwind_table[start][2])
            if ((addr - dif) < self.unwind_table[mid][0]) :
                stop = mid
            else :
                addr = addr - dif
                start = mid

        if self.unwind_table[start][0] <= addr :
            return self.unwind_table[start]
        else :
            return None

    def search_idx_3_0(self, addr) :
        first = 0
        last = len(self.unwind_table)
        while (first < last - 1) :
            mid = first + ((last - first + 1) >> 1)
            if (addr < self.unwind_table[mid][0]) :
                last = mid
            else :
                first = mid

        return self.unwind_table[first]

    def unwind_get_byte(self, ctrl) :

        if (ctrl.entries <= 0) :
            print_out_str("unwind: Corrupt unwind table")
            return 0

        val = self.ramdump.read_word(ctrl.insn)

        ret = (val >> (ctrl.byte * 8)) & 0xff

        if (ctrl.byte == 0) :
            ctrl.insn+=4
            ctrl.entries-=1
            ctrl.byte = 3
        else :
            ctrl.byte-=1

        return ret

    def unwind_exec_insn(self, ctrl, trace = False) :
        insn = self.unwind_get_byte(ctrl)

        if ((insn & 0xc0) == 0x00) :
            ctrl.vrs[SP] += ((insn & 0x3f) << 2) + 4
            if trace :
                print_out_str ("    add {0} to stack".format(((insn & 0x3f) << 2) + 4))
        elif ((insn & 0xc0) == 0x40) :
            ctrl.vrs[SP] -= ((insn & 0x3f) << 2) + 4
            if trace :
                print_out_str ("    subtract {0} from stack".format(((insn & 0x3f) << 2) + 4))
        elif ((insn & 0xf0) == 0x80) :
            vsp = ctrl.vrs[SP]
            reg = 4

            insn = (insn << 8) | self.unwind_get_byte(ctrl)
            mask = insn & 0x0fff
            if (mask == 0) :
                print_out_str ("unwind: 'Refuse to unwind' instruction")
                return -1

            # pop R4-R15 according to mask */
            load_sp = mask & (1 << (13 - 4))
            while (mask) :
                if (mask & 1) :
                    ctrl.vrs[reg] = self.ramdump.read_word(vsp)
                    if trace :
                        print_out_str ("    pop r{0} from stack".format(reg))
                    if ctrl.vrs[reg] is None :
                        return -1
                    vsp+=4
                mask >>= 1
                reg+=1
            if not load_sp :
                ctrl.vrs[SP] = vsp

        elif ((insn & 0xf0) == 0x90 and (insn & 0x0d) != 0x0d) :
            if trace :
                print_out_str ("    set SP with the value from {0}".format(insn & 0x0f))
            ctrl.vrs[SP] = ctrl.vrs[insn & 0x0f]
        elif ((insn & 0xf0) == 0xa0) :
            vsp = ctrl.vrs[SP]
            a = list(range(4,4 + (insn & 7)))
            a.append(4 + (insn & 7))
            # pop R4-R[4+bbb] */
            for reg in (a) :
                ctrl.vrs[reg] = self.ramdump.read_word(vsp)
                if trace :
                    print_out_str ("    pop r{0} from stack".format(reg))

                if ctrl.vrs[reg] is None :
                    return -1
                vsp+=4
            if (insn & 0x80) :
                if trace :
                    print_out_str ("    set LR from the stack")
                ctrl.vrs[14] = self.ramdump.read_word(vsp)
                if ctrl.vrs[14] is None :
                    return -1
                vsp+=4
            ctrl.vrs[SP] = vsp
        elif (insn == 0xb0) :
            if trace :
                print_out_str ("    set pc = lr")
            if (ctrl.vrs[PC] == 0) :
                ctrl.vrs[PC] = ctrl.vrs[LR]
            ctrl.entries = 0
        elif (insn == 0xb1) :
            mask = self.unwind_get_byte(ctrl)
            vsp = ctrl.vrs[SP]
            reg = 0


            if (mask == 0 or mask & 0xf0) :
                print_out_str ("unwind: Spare encoding")
                return -1

            # pop R0-R3 according to mask
            while mask :
                if (mask & 1) :
                    ctrl.vrs[reg] = self.ramdump.read_word(vsp)
                    if trace :
                        print_out_str ("    pop r{0} from stack".format(reg))
                    if ctrl.vrs[reg] is None :
                        return -1
                    vsp+=4
                mask >>= 1
                reg+=1
            ctrl.vrs[SP] = vsp
        elif (insn == 0xb2) :
            uleb128 = self.unwind_get_byte(ctrl)
            if trace :
                print_out_str ("    Adjust sp by {0}".format(0x204 + (uleb128 << 2)))

            ctrl.vrs[SP] += 0x204 + (uleb128 << 2)
        else :
            print_out_str ("unwind: Unhandled instruction")
            return -1

        return 0

    def prel31_to_addr(self, addr) :
        value = self.ramdump.read_word(addr)
        # offset = (value << 1) >> 1
        # C wants this sign extended. Python doesn't do that.
        # Sign extend manually.
        if (value & 0x40000000) :
            offset = value | 0x80000000
        else :
            offset = value

        # This addition relies on integer overflow
        # Emulate this behavior
        temp = addr + offset
        return (temp & 0xffffffff) + ((temp >> 32) & 0xffffffff)


    def unwind_frame(self, frame, trace = False) :
        low = frame.sp
        high = ((low + (THREAD_SIZE - 1)) & ~(THREAD_SIZE - 1)) + THREAD_SIZE
        idx = self.search_idx(frame.pc)

        if (idx is None) :
            if trace :
                print_out_str ("can't find %x" % frame.pc)
            return -1

        ctrl = UnwindCtrlBlock()
        ctrl.vrs[FP] = frame.fp
        ctrl.vrs[SP] = frame.sp
        ctrl.vrs[LR] = frame.lr
        ctrl.vrs[PC] = 0

        if (idx[1] == 1) :
            return -1

        elif ((idx[1] & 0x80000000) == 0) :
            ctrl.insn = self.prel31_to_addr(idx[2]+4)

        elif (idx[1] & 0xff000000) == 0x80000000 :
            ctrl.insn = idx[2]+4
        else :
            print_out_str ("not supported")
            return -1

        val = self.ramdump.read_word(ctrl.insn)

        if ((val & 0xff000000) == 0x80000000) :
            ctrl.byte = 2
            ctrl.entries = 1
        elif ((val & 0xff000000) == 0x81000000) :
            ctrl.byte = 1
            ctrl.entries = 1 + ((val & 0x00ff0000) >> 16)
        else :
            return -1

        while (ctrl.entries > 0) :
            urc = self.unwind_exec_insn(ctrl, trace)
            if (urc < 0) :
                return urc
            if (ctrl.vrs[SP] < low or ctrl.vrs[SP] >= high) :
                return -1

        if (ctrl.vrs[PC] == 0) :
            ctrl.vrs[PC] = ctrl.vrs[LR]

        # check for infinite loop */
        if (frame.pc == ctrl.vrs[PC]) :
            return -1

        frame.fp = ctrl.vrs[FP]
        frame.sp = ctrl.vrs[SP]
        frame.lr = ctrl.vrs[LR]
        frame.pc = ctrl.vrs[PC]

        return 0

    def unwind_backtrace(self, sp, fp, pc, lr, extra_str  = "", out_file = None, trace = False) :
        offset = 0
        frame = Stackframe(fp, sp, lr, pc)
        frame.fp = fp
        frame.sp = sp
        frame.lr = lr
        frame.pc = pc

        while True :
            where = frame.pc
            offset = 0

            r = self.ramdump.unwind_lookup(frame.pc)
            if r is None :
                symname = "UNKNOWN"
                offset = 0x0
            else :
                symname, offset = r
            pstring = (extra_str+"[<{0:x}>] {1}+0x{2:x}".format(frame.pc, symname, offset))
            if out_file :
                out_file.write (pstring+"\n")
            else :
                print_out_str (pstring)

            urc = self.unwind_frame(frame, trace)
            if urc < 0 :
                break


