#!/bin/sh

process=$(nproc)

# copy modified files to source
cp -r src/systemcall linux/custom_systemcall

# build
cd linux
sudo make menuconfig -j$process
sudo make -j$process

# install
sudo make modules_install -j$process
sudo make install -j$process
sudo update-grub
Reboot
