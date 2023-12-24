cmake_minimum_required(VERSION 3.27)

set(TARGET_NAME kernel)
set(CMAKE_C_COMPILER_WORKS TRUE)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DLITTLEFS -DFS_USE_SLFS -D_ENABLE_SMP_")
set(CMAKE_EXECUTABLE_SUFFIX .elf)

project(
  ${TARGET_NAME} 
  VERSION 1.0
  LANGUAGES ASM C
)

include_directories(inc)

add_compile_options(
  -g -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard
  -Wall -fno-omit-frame-pointer -ffreestanding
)

add_subdirectory(core)
add_subdirectory(arch)
add_subdirectory(drivers)
add_subdirectory(fs)
add_subdirectory(libs)

add_executable(
  ${TARGET_NAME}
  $<TARGET_OBJECTS:arch_objs>
  $<TARGET_OBJECTS:core_objs>
  $<TARGET_OBJECTS:drivers_objs>
  $<TARGET_OBJECTS:fs_objs>
  $<TARGET_OBJECTS:libs_objs>
)

target_link_options(
  ${TARGET_NAME} PRIVATE
  -nostdlib
  -Wl,-Map=${TARGET_NAME}.map
  -Wl,-T
  -Wl,${CMAKE_CURRENT_SOURCE_DIR}/linker/${TARGET_NAME}.lds
)
