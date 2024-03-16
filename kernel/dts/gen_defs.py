#!/usr/bin/env python3

# SPDX-License-Identifier: MIT OR Apache-2.0
#
# Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

import glob
import os
import re

class node:
    def __init__(self, parent, name, compat, base_addr, intr):
        self.name = name
        self.compat = compat
        self.base_addr = base_addr
        self.intr = intr
        self.parent = parent
        self.leaves = []

    def add_leaf(self, leaf):
        self.leaves.append(leaf)

def dfs(node):
    for leaf in node.leaves:
        dfs(leaf)
    print(f"node: {node.name}\n")

def build_tree(dts_file, root):
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
                leaf = node(current, node_name, None, None, None)
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

            elif in_devicetree and 'compatible' in line and ';' in line:
                line = line.replace(';', '').replace('\"', '')
                line_split = line.split('=')
                compat_str = line_split[-1].strip()
                compat_str = compat_str.replace('.', '_').replace(' ', '_').replace('-', '_').replace(',', '_')
                current.compat = compat_str

            elif in_devicetree and 'reg' in line:
                left = line.find('<')
                right = line.find('>')
                regs = line[left + 1:right].split(' ')
                current.base_addr = regs[0].strip()

            elif in_devicetree and 'interrupts' in line:
                left = line.find('<')
                right = line.find('>')
                intr = line[left + 1:right].split(' ')
                current.intr = intr[0].strip()


def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    dts_path = os.path.join(script_dir, '*.dts*')

    root = node(None, None, None, None, None)

    for file_name in glob.glob(dts_path, recursive=False):
        print(f"filename: {file_name}")
        build_tree(file_name, root)

    dfs(root)

if __name__ == '__main__':
    main()