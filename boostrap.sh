# install necessary packages for building linux kernel
sudo apt update
sudo apt upgrade -y
sudo apt install ccache git fakeroot build-essential ncurses-dev xz-utils libssl-dev bc flex libelf-dev bison -y

# download linux kernel
sudo apt install wget -y
wget https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-6.1.tar.xz
tar xvf linux-6.1.tar.xz
mv linux-6.1 linux

# copy default config from system
cp -v /boot/config-$(uname -r) ./linux/.config
