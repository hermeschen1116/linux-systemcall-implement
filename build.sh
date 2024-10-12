# copy modified files to source
cp src/* linux/arch/x86/kernel/*.c

# build
sudo make menuconfig -j8 -C linux -i linux
sudo make arch=$(arch) -j8 -C linux -i linux
