sudo make menuconfig -j8 -C linux -i linux
sudo make arch=$(arch) -j8 -C linux -i linux
