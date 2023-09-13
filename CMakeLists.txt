cmake_minimum_required(VERSION 3.16)

set(CMAKE_C_COMPILER "arm-none-eabi-gcc")
set(CMAKE_ASM_COMPILER "arm-none-eabi-as")
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
set(CMAKE_LINKER "arm-none-eabi-gcc")
#set(CMAKE_LINKER "arm-none-eabi-ld")
enable_language(C ASM)

#set(CMAKE_TOOLCHAIN_FILE cross-tools.cmake)

project(slos VERSION 1.0)

#configure_file(slos_config.h.in slos_config.h)

add_subdirectory(kernel)