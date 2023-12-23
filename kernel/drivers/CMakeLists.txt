add_library(
  drivers_objs OBJECT
  dma/dma.c
  odev/odev.c
  ramdisk/ramdisk_io.c
  uart/rwbyte.c
)

#target_compile_options(drivers_objs PRIVATE -Wall -fno-omit-frame-pointer -ffreestanding)
