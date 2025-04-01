#!/usr/bin/env bash
set -e

version=$(git describe --tags --always)
build_type=${build_type:-release}

if [[ "$build_type" == "release" ]]; then
    version_suffix=""
else
    version_suffix="-$build_type"
fi

export ARCH=arm
export CROSS_COMPILE=arm-linux-gnueabihf-
export LOCALVERSION=-$version$version_suffix

MAKE_OPTS="-j$(nproc --ignore 2) O=kernel_out"

if [[ $# -gt 0 ]]; then
    make $MAKE_OPTS "$@"
else
    make $MAKE_OPTS imsar_grizzly_defconfig
    if [[ "$build_type" != "release" ]]; then
        while read -r line; do
            ./scripts/config --file kernel_out/.config $line
        done < imsar_config_$build_type
        make $MAKE_OPTS oldconfig
    fi
    make $MAKE_OPTS uImage UIMAGE_LOADADDR=0x8000
    make $MAKE_OPTS modules
    make $MAKE_OPTS modules_install INSTALL_MOD_PATH=$PWD/modules_out
fi


