cmake_minimum_required(VERSION 3.27)

#include(${CMAKE_CURRENT_SOURCE_DIR}/arm-none-eabi.cmake)

enable_language(ASM C)

project(
  kernel 
  VERSION 1.0
  LANGUAGES C
)

include_directories(inc)

add_subdirectory(core)
add_subdirectory(arch)
add_subdirectory(drivers)
add_subdirectory(fs)
add_subdirectory(libs)

set(CMAKE_EXECUTABLE_SUFFIX .elf)

add_executable(
  kernel
  $<TARGET_OBJECTS:arch_objs>
  $<TARGET_OBJECTS:core_objs>
  $<TARGET_OBJECTS:drivers_objs>
  $<TARGET_OBJECTS:fs_objs>
  $<TARGET_OBJECTS:libs_objs>
)


target_link_options(
  kernel PUBLIC
  -Wl,-T
  -Wl,${CMAKE_CURRENT_SOURCE_DIR}/linker/kernel.lds
)
