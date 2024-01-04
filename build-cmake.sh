#!/bin/bash

source setup-cmake.sh

# Function to display usage information
usage() {
    echo "Usage: $0 [options...]"
    echo "Options:"
    echo "  -a    build all targets"
    echo "  -c    clean builds"
    echo "  -p    build apps"
    echo "  -r    build ramdisk"
    echo "  -k    build kernel32"
    echo "  -l    build kernel64"
    echo "  -h    show help message"
}

build_apps() {
    echo "cmake build for apps"
    cd ${SLOS_PATH}
    rm -rf build-apps
    mkdir build-apps && cd build-apps
    cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=${SLOS_PATH}/cmake/arm-none-eabi.cmake ../apps
    make
}

clean_builds() {
    if [[ -z "${SLOS_PATH}" ]]
        then
        echo "SLOS_PATH sin't set"
    else 
        echo "clean ${SLOS_PATH}/build-slos, mkfs, apps and ramdisk"
        rm -rf ${SLOS_PATH}/build-slos
        rm -rf ${SLOS_PATH}/build-mkfs
        rm -rf ${SLOS_PATH}/build-apps
        rm -rf ${SLOS_PATH}/kernel/ramdisk
    fi
}

build_ramdisk() {
    APPS_FILE=${SLOS_PATH}/build-apps/helloworld
    if [[ ! -f ${APPS_FILE} ]]; then
        echo "build apps first"
        exit 1
    fi

    echo "cmake build for mkfs"
    cd ${SLOS_PATH}
    rm -rf build-mkfs
    mkdir build-mkfs && cd build-mkfs
    cmake ../mkfs
    make

    echo "build ramdisk.img"
    rm -rf ${SLOS_PATH}/kernel/ramdisk
    mkdir ${SLOS_PATH}/kernel/ramdisk
    cp ${SLOS_PATH}/build-apps/helloworld ${SLOS_PATH}/kernel/ramdisk
    cp ${SLOS_PATH}/build-mkfs/mkfs ${SLOS_PATH}/kernel/ramdisk
    cd ${SLOS_PATH}/kernel/ramdisk
    ./mkfs ./ramdisk.img ./helloworld
}

build_kernel32() {
    echo "cmake build for kernel32"
    cd ${SLOS_PATH}
    rm -rf build-slos
    mkdir build-slos && cd build-slos
    cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=${SLOS_PATH}/cmake/arm-none-eabi.cmake ..
    make
}

build_kernel64() {
    echo "cmake build for kernel64"
    cd ${SLOS_PATH}
    rm -rf build-slos
    mkdir build-slos && cd build-slos
    cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=${SLOS_PATH}/cmake/aarch64-none-elf.cmake -DTARGET_ARCH="aarch64" ..
    make
}

# Parse command-line options
while getopts 'acprklh' option; do
    case "$option" in
        a) 
            build_apps
            build_ramdisk
            build_kernel32
            ;;
        c) 
            clean_builds
            ;;
        p) 
            build_apps
            ;;
        r) 
            build_ramdisk
            ;;
        k) 
            build_kernel32
            ;;
        l) 
            build_kernel64
            ;;
        h) 
            usage
            exit
            ;;
        *) 
            usage
            exit 1
            ;;
    esac
done

# Check if no options were provided
if [ $OPTIND -eq 1 ]; then
    echo "No options specified"
    usage
    exit 1
fi

