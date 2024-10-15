#!/bin/sh

process=$(nproc)
architecture=x86_64

# copy modified files to source
cp src/systemcall/* linux/custom_systemcall/

cd linux
# build
sudo make menuconfig LLVM=1 CC="ccache clang" -j$process O=./build
sudo make LLVM=1 CC="ccache clang" Arch=$architecture -j$process O=./build

# install
sudo make modules_install -j$process O=./build
sudo make install -j$process O=./build
sudo update-grub
