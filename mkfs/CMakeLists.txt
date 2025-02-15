cmake_minimum_required(VERSION 3.27)

set(TARGET_NAME mkfs)

project(
  ${TARGET_NAME}
  VERSION 1.0
  LANGUAGES CXX
)

add_executable(
  ${TARGET_NAME}
  mkfs.cpp
)

target_compile_features(${TARGET_NAME} PRIVATE cxx_std_17)
