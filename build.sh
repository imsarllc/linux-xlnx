#!/bin/bash
set -e
nthreads=$(nproc --ignore=1)
export ARCH=arm
make xilinx_zynq_defconfig
make -j$nthreads uImage UIMAGE_LOADADDR=0x8000
make modules -j$nthreads
make modules_install INSTALL_MOD_PATH=usr/
