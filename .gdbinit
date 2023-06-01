target remote localhost:3000
file out/kernel/kernel.elf
b ssbl
b create_ramdisk_fs
b alloc_datablk
