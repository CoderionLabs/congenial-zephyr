#!/bin/bash
set -evu

PREFIX=/usr/local


if [ "$OS" == "arch" ] || [ "$OS" == "fedora" ]
then
	PREFIX=/usr
fi

export LD_LIBRARY_PATH=$PREFIX/lib
export CPLUS_INCLUDE_PATH=$PREFIX/include
export CURRENT_PATH=.
mkdir $CPLUS_INCLUDE_PATH/zephyr
mkdir $CPLUS_INCLUDE_PATH/sibe

# Install OpenDHT dependencies
apt-get -y install libncurses5-dev libreadline-dev libjsoncpp-dev nettle-dev libgnutls-dev libjsoncpp-dev

# Install python binding dependencies
apt-get -y install cython3 python3-dev python3-setuptools

# Build and install msgpack-c
apt-get -y install build-essential cmake
wget https://github.com/msgpack/msgpack-c/releases/download/cpp-2.1.1/msgpack-2.1.1.tar.gz
tar -xzf msgpack-2.1.1.tar.gz
cd msgpack-2.1.1 && mkdir build && cd build
cmake -DMSGPACK_CXX11=ON -DMSGPACK_BUILD_EXAMPLES=OFF -DCMAKE_INSTALL_PREFIX=/usr ..
make -j4
make install
cd ../../

git clone https://github.com/savoirfairelinux/opendht.git
cd opendht
mkdir build && cd build
cmake -DOPENDHT_PYTHON=ON -DCMAKE_INSTALL_PREFIX=/usr ..
make -j4
make install
cd ../../

git clone https://github.com/DokuEnterprise/crypto
cd crypto
cd stanfordibe
cp /app/congenial-zephyr/src/Network/Server/docker/libssl.so.1.0.0 .
make
make install
cd ../../

wget http://www.openssl.org/source/openssl-1.0.0a.tar.gz
tar -xf openssl-1.0.0a.tar.gz
cd openssl-1.0.0a
./config --prefix=/usr/local/openssl-1.0
make
make install_sw
ls
cd ../
ls
cd congenial-zephyr
cd src

cd Mixer
make
make install
cd ../
cd PKG
make
make install




CMD ["/bin/bash"]