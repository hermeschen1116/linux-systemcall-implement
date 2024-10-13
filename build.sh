#!/bin/sh
# copy modified files to source
cp src/*.c linux/arch/x86/kernel/

# build
cd linux
sudo make menuconfig -j$(nproc)
sudo make -j$(nproc)

# install
sudo make modules_install -j$(nproc)
sudo make install -j$(nproc)
