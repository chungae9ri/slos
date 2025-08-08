#!/usr/bin/env python3

# SPDX-License-Identifier: MIT OR Apache-2.0
#
# Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

import glob
import os
import argparse

compat_dict = {}

class node:
    def __init__(self, parent=None, name=None, idx=None, compat=None, base_addr=None, intr=None, clkfreq=None):
        self.name = name
        self.idx = idx
        self.compat = compat
        self.base_addr = base_addr
        self.intr = intr
        self.clkfreq = None
        self.parent = parent
        self.leaves = []

    def add_leaf(self, leaf):
        self.leaves.append(leaf)

def handle_compat(line, current):
    line = line.replace(';', '').replace('\"', '')
    line_split = line.split('=')
    compat_str = line_split[-1].strip()
    compat_str = compat_str.replace('.', '_').replace(' ', '_').replace('-', '_').replace(',', '_')
    current.compat = compat_str
    if compat_str in compat_dict:
        compat_dict[compat_str] = compat_dict[compat_str] + 1
    else:
        compat_dict[compat_str] = 0

    current.idx = compat_dict[compat_str]


def handle_reg(line, current):
    left = line.find('<')
    right = line.find('>')
    regs = line[left + 1:right].split(' ')
    current.base_addr = regs[0].strip()

def handle_intr(line, current):
    left = line.find('<')
    right = line.find('>')
    intr = line[left + 1:right].split(' ')
    current.intr = intr[0].strip()

def handle_clkfreq(line, current):
    left = line.find('<')
    right = line.find('>')
    clkfreq = line[left + 1:right].split(' ')
    current.clkfreq = clkfreq[0].strip()

def dfs(node, path):
    for leaf in node.leaves:
        devtree_macro = path +"_S_" + leaf.name
        compat = str(leaf.compat).upper()
        #print(f"#define {devtree_macro} 1")
        if leaf.compat is not None:
            print(f"#define {compat}_{leaf.idx}_P_COMPAT \"{compat}\"")
        if leaf.base_addr is not None:
            print(f"#define {compat}_{leaf.idx}_P_BASE_ADDR {leaf.base_addr}")
        if leaf.intr is not None:
            print(f"#define {compat}_{leaf.idx}_P_INTR {leaf.intr}")
        if leaf.clkfreq is not None:
            print(f"#define {compat}_{leaf.idx}_P_CLKFREQ {leaf.clkfreq}")

        dfs(leaf, devtree_macro)

def build_tree(dts_file, root):
    handlers = {'compatible': handle_compat, 
                'reg': handle_reg,
                 'interrupts': handle_intr,
                 'clkfreq': handle_clkfreq
                }

    with open(dts_file, 'r') as file:
        in_devicetree = False
        current = root
        depth = 0
        for line in file:
            if line.find('/') == 0 and line.find('{') == 2:
                in_devicetree = True
                root.name = "DT_N"
                depth += 1

            elif in_devicetree and '{' in line:
                if ':' in line:
                    left = line.find(':') + 1
                else:
                    left = 0
                right = line.find('{')
                node_name = line[left:right].strip()
                node_name = node_name.replace('@', '_')
                leaf = node(current, node_name, None, None, None, None)
                current.add_leaf(leaf)
                current = leaf
                depth += 1

            elif in_devicetree and '}' in line and ';' in line:
                depth -= 1
                if depth == 0:
                    in_devicetree = False
                    current = None
                else:
                    current = current.parent
            else:
                for key, handler in handlers.items():
                    if key in line:
                        handler(line, current)
                        break

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-s', '--soc', type=str, required=True, default='zynq7000')
    args = parser.parse_args()

    soc = args.soc

    script_dir = os.path.dirname(os.path.abspath(__file__)) + '/../dts/' + soc 
    dts_path = os.path.join(script_dir, '*.dts*')

    root = node(None, None, None, None, None, None)

    for file_name in glob.glob(dts_path, recursive=False):
        #print(f"filename: {file_name}")
        build_tree(file_name, root)

    dfs(root, root.name)

if __name__ == '__main__':
    main()