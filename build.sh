#!/usr/bin/env bash
export ARCH=arm
export CROSS_COMPILE=arm-linux-gnueabihf-
MAKE_OPTS="-j$(nproc --ignore 2) O=kernel_out"
make mrproper
make $MAKE_OPTS xilinx_zynq_defconfig
make $MAKE_OPTS uImage UIMAGE_LOADADDR=0x8000
make $MAKE_OPTS modules
make $MAKE_OPTS modules_install INSTALL_MOD_PATH=./
