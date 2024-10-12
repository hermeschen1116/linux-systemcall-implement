#!/bin/sh
# install necessary packages for building linux kernel
sudo apt update
sudo apt upgrade -y
sudo apt install ccache git fakeroot build-essential ncurses-dev xz-utils libssl-dev bc flex libelf-dev bison -y

# download linux kernel
if [ ! -d "linux" ]
then
	sudo apt install wget -y
	wget https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-6.1.tar.xz
	tar xvf linux-6.1.tar.xz
	mv linux-6.1 linux
	rm linux-6.1.tar.xz
fi

# copy default config from system
cp -v /boot/config-$(uname -r) ./linux/.config

# link config
if [ ! -L "src/syscalls.h" ]
then
	ln -s ../linux/include/linux/syscalls.h src/syscalls.h
fi

if [ ! -L "src/syscall_64.tbl" ]
then
ln -s ../linux/arch/x86/entry/syscalls/syscall_64.tbl src/syscall_64.tbl
fi

if [ ! -L "src/Makefile" ]
then
ln -s ../linux/arch/x86/kernel/Makefile src/Makefile
if
