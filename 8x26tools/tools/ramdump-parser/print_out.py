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

out_file = None

def set_outfile(path) :
    global out_file
    out_file = open(path,"wb")

def print_out_str(string) :
    if out_file is None :
        print (string)
    else :
        out_file.write((string+"\n").encode('ascii','ignore'))

# LGE_CHANGE_S: Add print_out_str_uni function for logger parser
def print_out_str_uni(string) :
    if out_file is None :
        print (string)
    else :
        out_file.write(string+"\n")
# LGE_CHANGE_E
