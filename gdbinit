target remote localhost:3000
file out/kernel/kernel.elf
b ssbl
b create_ramdisk_fs
b alloc_datablk
# to run a command when breakpoint 1 is hit
#commands 1
#restore a.bin binary 0x1000
#continue
#end
# dump a memory from target address start end and save it to memdump.bin
#dump binary memory memdump.bin 0x100000 0x108000
