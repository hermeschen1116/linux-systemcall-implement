#!/bin/sh

process=$(nproc)
architecture=$(arch)

cd linux

# copy default config from system
sudo make clean -j$process
cp -v /boot/config-$(uname -r) ./linux/.config

# copy modified files to source
cp src/systemcall/* linux/custom_systemcall/

# build
sudo make menuconfig -j$process O=./build
sudo make Arch=$architecture -j$process O=./build
sudo make bzImage Arch=$architecture -j$process O=./build
sudo make modules Arch=$architecture -j$process O=./build

# install
sudo make modules_install -j$process O=./build
sudo make install -j$process O=./build
sudo update-grub
