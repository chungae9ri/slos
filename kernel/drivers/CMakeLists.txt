if (TARGET_ARCH STREQUAL "aarch64")
  add_library(
    drivers_objs OBJECT
    uart/uart.c
  )
else ()
  add_library(
    drivers_objs OBJECT
    dma/dma.c
    odev/odev.c
    ramdisk/ramdisk_io.c
    uart/uart.c
  )
  add_subdirectory(tinyusb/src)
  tinyusb_target_add(drivers_objs)
endif ()
