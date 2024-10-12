# copy modified files to source
cp src/Makefile linux/arch/x86/kernel/
cp src/syscall_64.tbl linux/arch/x86/entry/syscalls/
cp src/syscalls.h linux/include/linux/
cp src/systemcall/* linux/arch/x86/kernel/

# build
sudo make menuconfig -j8 -C linux -i linux
sudo make arch=$(arch) -j8 -C linux -i linux
