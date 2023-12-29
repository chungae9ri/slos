cmake_minimum_required(VERSION 3.27)

set(TARGET_NAME slos)
set(CMAKE_C_COMPILER_WORKS TRUE)

project(
  ${TARGET_NAME}
  VERSION 1.0
  LANGUAGES ASM C
)

add_library(
  ${TARGET_NAME}
  STATIC
  print_mesg.c
  syscall.S
)

target_compile_options(
  ${TARGET_NAME} PRIVATE
  -g -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard
  -Wall -fno-omit-frame-pointer -ffreestanding
)
