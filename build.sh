#!/bin/sh

process="$(nproc)"
architecture="x86_64"
source="linux"
build_dir="./build"

# copy modified files to source
cp src/systemcall/* linux/custom_systemcall/

# build
KBUILD_BUILD_TIMESTAMP="" sudo make menuconfig CC="ccache gcc" -j$process -C$source O=$build_dir
KBUILD_BUILD_TIMESTAMP="" sudo make CC="ccache gcc -O3" Arch=$architecture -j$process -C$source O=$build_dir 2>&1 | tee build.log

# install
sudo make modules_install -j$process -C$source O=$build_dir
sudo make install -j$process -C$source O=$build_dir
sudo update-grub
