set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_C_COMPILER "arm-none-eabi-gcc")
set(CMAKE_ASM_COMPILER "arm-none-eabi-gcc")
set(CMAKE_LINKER "arm-none-eabi-gcc")
set(CMAKE_CROSSCOMPILING true)
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DLITTLEFS -DFS_USE_SLFS -D_ENABLE_SMP_")

add_compile_options(
  -g -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard
  -Wall -fno-omit-frame-pointer -ffreestanding
)

add_link_options(
  -nostdlib 
)

