#!/bin/bash

source setup-cmake.sh

echo "cmake build for mkfs"
rm -rf build-mkfs
mkdir build-mkfs && cd build-mkfs
cmake ../mkfs
make

echo "cmake build for apps"
cd ${SLOS_PATH}
rm -rf build-apps
mkdir build-apps && cd build-apps
cmake -DCMAKE_TOOLCHAIN_FILE=${SLOS_PATH}/cmake/arm-none-eabi.cmake ../apps
make

echo "build ramdisk.img"
rm -rf ${SLOS_PATH}/kernel/ramdisk
mkdir ${SLOS_PATH}/kernel/ramdisk
cp ${SLOS_PATH}/build-apps/helloworld ${SLOS_PATH}/kernel/ramdisk
cp ${SLOS_PATH}/build-mkfs/mkfs ${SLOS_PATH}/kernel/ramdisk
cd ${SLOS_PATH}/kernel/ramdisk
./mkfs ./ramdisk.img ./helloworld

echo "cmake build for slos"
cd ${SLOS_PATH}
rm -rf build-slos
mkdir build-slos && cd build-slos
cmake -DCMAKE_TOOLCHAIN_FILE=${SLOS_PATH}/cmake/arm-none-eabi.cmake ..
make

