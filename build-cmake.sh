#!/bin/bash

source setup-cmake.sh

BUILD_PATH=${SLOS_PATH}/build
if [[ ! -d "${BUILD_PATH}" ]]
then
    mkdir ${BUILD_PATH}
fi

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
    echo "build_path ${BUILD_PATH}"
    cd ${BUILD_PATH}
    rm -rf ${BUILD_PATH}/apps
    mkdir ${BUILD_PATH}/apps && cd ${BUILD_PATH}/apps
    cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=${SLOS_PATH}/cmake/arm-none-eabi.cmake ${SLOS_PATH}/apps
    make
}

clean_builds() {
    if [[ -z "${SLOS_PATH}" ]]
        then
        echo "SLOS_PATH sin't set"
    else 
        echo "clean ${BUILD_PATH}"
        rm -rf ${BUILD_PATH}
    fi
}

build_ramdisk() {
    APPS_FILE=${BUILD_PATH}/apps/helloworld
    if [[ ! -f ${APPS_FILE} ]]; then
        echo "build apps first"
        exit 1
    fi

    echo "cmake build for mkfs"
    cd ${BUILD_PATH}
    rm -rf ${BUILD_PATH}/mkfs
    mkdir ${BUILD_PATH}/mkfs && cd ${BUILD_PATH}/mkfs
    cmake ${SLOS_PATH}/mkfs
    make

    echo "build ramdisk.img"
    rm -rf ${SLOS_PATH}/kernel/ramdisk
    mkdir ${SLOS_PATH}/kernel/ramdisk
    cp ${BUILD_PATH}/apps/helloworld ${SLOS_PATH}/kernel/ramdisk
    cp ${BUILD_PATH}/mkfs/mkfs ${SLOS_PATH}/kernel/ramdisk
    cd ${SLOS_PATH}/kernel/ramdisk
    ./mkfs ./ramdisk.img ./helloworld
}

build_kernel32() {
    echo "cmake build for kernel32"
    cd ${BUILD_PATH}
    rm -rf ${BUILD_PATH}/slos32
    mkdir ${BUILD_PATH}/slos32 && cd ${BUILD_PATH}/slos32
    cmake -DCMAKE_TOOLCHAIN_FILE=${SLOS_PATH}/cmake/arm-none-eabi.cmake ${SLOS_PATH}
    make
}

build_kernel64() {
    echo "cmake build for kernel64"
    cd ${BUILD_PATH}
    rm -rf ${BUILD_PATH}/slos64
    mkdir ${BUILD_PATH}/slos64 && cd ${BUILD_PATH}/slos64
    cmake -DCMAKE_TOOLCHAIN_FILE=${SLOS_PATH}/cmake/aarch64-none-elf.cmake ${SLOS_PATH}
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

