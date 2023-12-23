cmake_minimum_required(VERSION 3.27)

#set(CMAKE_TOOLCHAIN_FILE cross-tools.cmake)

project(slos VERSION 1.0)

#configure_file(slos_config.h.in slos_config.h)

add_subdirectory(kernel)