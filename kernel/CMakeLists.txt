cmake_minimum_required(VERSION 3.27)

#include(${CMAKE_CURRENT_SOURCE_DIR}/arm-none-eabi.cmake)

enable_language(ASM C)
set(TARGET_NAME kernel)

project(
  ${TARGET_NAME} 
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
  ${TARGET_NAME}
  $<TARGET_OBJECTS:arch_objs>
  $<TARGET_OBJECTS:core_objs>
  $<TARGET_OBJECTS:drivers_objs>
  $<TARGET_OBJECTS:fs_objs>
  $<TARGET_OBJECTS:libs_objs>
)

target_link_options(
  ${TARGET_NAME} PUBLIC
  -Wl,-Map=${TARGET_NAME}.map
  -Wl,-T
  -Wl,${CMAKE_CURRENT_SOURCE_DIR}/linker/${TARGET_NAME}.lds
)
