#!/usr/bin/env bash
set -e

export ARCH=arm
export CROSS_COMPILE=arm-linux-gnueabihf-

MAKE_OPTS="-j$(nproc --ignore 2) O=kernel_out"

if [[ -n "$1" ]]; then
    make $MAKE_OPTS $1
else
    make $MAKE_OPTS imsar_grizzly_defconfig
    make $MAKE_OPTS uImage UIMAGE_LOADADDR=0x8000
    make $MAKE_OPTS modules
    make $MAKE_OPTS modules_install INSTALL_MOD_PATH=$PWD/modules_out
fi


