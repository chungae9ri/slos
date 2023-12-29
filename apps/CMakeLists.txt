cmake_minimum_required(VERSION 3.27)

set(TARGET_NAME helloworld)
set(CMAKE_C_COMPILER_WORKS TRUE)

project(
  ${TARGET_NAME}
  VERSION 1.0
  LANGUAGES C
)

include(ExternalProject)

ExternalProject_Add(
  libslos
  SOURCE_DIR $ENV{SLOS_PATH}/libslos
  CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
  INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_if_different
        <BINARY_DIR>/libslos.a
        ${CMAKE_BINARY_DIR}/libslos.a
)

link_directories(${CMAKE_BINARY_DIR})

add_executable(
  ${TARGET_NAME}
  helloworld.c
)

target_compile_options(
  ${TARGET_NAME} PRIVATE
  -g -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard
  -Wall -fno-omit-frame-pointer -ffreestanding
)

target_link_libraries(
  ${TARGET_NAME} PRIVATE
  libslos.a
)

target_link_options(
  ${TARGET_NAME} PRIVATE
  -e main
  -nostdlib
  -Wl,-Map=${TARGET_NAME}.map
  -static
)
