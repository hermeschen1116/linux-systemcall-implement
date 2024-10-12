#!/bin/sh
# copy modified files to source
cp src/*.c linux/arch/x86/kernel/

# build
sudo make menuconfig -j8 -C linux -i linux
sudo make arch=$(arch) -j8 -C linux -i linux
