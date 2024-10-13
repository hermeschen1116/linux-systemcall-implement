#!/bin/sh
# copy modified files to source
cp src/*.c linux/arch/x86/kernel/

# build
cd linux
sudo make menuconfig -j8
sudo make bzImage arch=$(arch) -j8　

# install
sudo make modules_install -j8
sudo make install -j8
