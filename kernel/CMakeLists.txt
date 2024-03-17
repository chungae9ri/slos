cmake_minimum_required(VERSION 3.27)

if (TARGET_ARCH STREQUAL "aarch64")
set(TARGET_NAME kernel64)
else ()
set(TARGET_NAME kernel32)
endif ()

set(CMAKE_C_COMPILER_WORKS TRUE)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DLITTLEFS -DFS_USE_SLFS -D_ENABLE_SMP_")
set(CMAKE_EXECUTABLE_SUFFIX .elf)

project(
  ${TARGET_NAME} 
  VERSION 1.0
  LANGUAGES ASM C
)

set(devicetree_defs_fname generated_devicetree_defs.h)
set(devicetree_custom_target gen_devicetree_defs)
add_custom_command(
  OUTPUT ${devicetree_defs_fname}
  COMMAND ${CMAKE_COMMAND} -E env python3 ${CMAKE_CURRENT_SOURCE_DIR}/dts/gen_defs.py > ${devicetree_defs_fname}
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/dts/gen_defs.py
)

add_custom_target(
  ${devicetree_custom_target} ALL
  DEPENDS ${devicetree_defs_fname}
)

include_directories(inc)

if (TARGET_ARCH STREQUAL "aarch64")
message("TARGET_ARCH is aarch64")
add_compile_options(
  -g -march=armv8-a -mcpu=cortex-a53
  -Wall -fno-omit-frame-pointer -ffreestanding
)

add_subdirectory(arch/ca-53)
add_subdirectory(core)
add_subdirectory(core64)
add_subdirectory(drivers)

add_executable(
  ${TARGET_NAME}
  $<TARGET_OBJECTS:arch_objs>
  $<TARGET_OBJECTS:core64_objs>
  $<TARGET_OBJECTS:core_objs>
  $<TARGET_OBJECTS:drivers_objs>
)

else ()
message("TARGET_ARCH is aarch32")
add_compile_options(
  -g -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard
  -Wall -fno-omit-frame-pointer -ffreestanding
)

add_subdirectory(core)
add_subdirectory(arch/ca-9)
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
endif ()

add_dependencies(${TARGET_NAME} ${devicetree_custom_target})

target_link_options(
  ${TARGET_NAME} PRIVATE
  -nostdlib
  -Wl,-Map=${TARGET_NAME}.map
  -Wl,-T
  -Wl,${CMAKE_CURRENT_SOURCE_DIR}/linker/${TARGET_NAME}.lds
)
