Require g++ (Ubuntu 7.3.0-16ubuntu3) 7.3.0

# How to install g++ 7:

## install packages:
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt update
sudo apt install g++-7 -y

## set up the symbolic links:
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-7 60 \
                         --slave /usr/bin/g++ g++ /usr/bin/g++-7
sudo update-alternatives --config gcc

## check with:
gcc --version
g++ --version
