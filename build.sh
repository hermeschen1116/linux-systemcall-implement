#!/bin/sh

process="$(nproc)"
architecture="x86_64"
source="linux"
build_dir="./build"
cc="ccache gcc"

# copy modified files to source
cp src/systemcall/* linux/custom_systemcall/

# build
KBUILD_BUILD_TIMESTAMP="" sudo make menuconfig CC=$cc -j$process -C$source O=$build_dir
KBUILD_BUILD_TIMESTAMP="" sudo make CC=$cc Arch=$architecture -j$process -C$source O=$build_dir

# install
sudo make modules_install -j$process -C$source O=$build_dir
sudo make install -j$process -C$source O=$build_dir
sudo update-grub
