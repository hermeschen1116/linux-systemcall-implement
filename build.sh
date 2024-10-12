cd linux
sudo make menuconfig -j8
sudo make arch=$(arch) -j8
cd ..
