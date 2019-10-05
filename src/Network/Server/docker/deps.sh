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

apt-get update
# Install OpenDHT dependencies
apt-get -y install libncurses5-dev libreadline-dev libjsoncpp-dev nettle-dev libgnutls-dev libjsoncpp-dev

# Install python binding dependencies
apt-get -y install cython3 python3-dev python3-setuptools

# Install boost
wget -O boost_1_70_0.tar.gz https://dl.bintray.com/boostorg/release/1.70.0/source/boost_1_70_0.tar.gz
tar xzvf boost_1_70_0.tar.gz
cd boost_1_70_0/
./bootstrap.sh --prefix=/usr/local
./b2
./b2 install
cd ../

# Install boost beast
#git clone https://github.com/mutexunlocked/boost boostdems
#cd boostdems && cd include && cd boost
#mkdir -p $PREFIX/include/boost
#rm -R $PREFIX/include/boost/beast
#rm $PREFIX/include/boost/beast.hpp
#cp -R beast $PREFIX/include/boost
#cp beast.hpp $PREFIX/include/boost
#cd ../../../


# Build and install msgpack-c
apt-get -y install build-essential cmake
wget https://github.com/msgpack/msgpack-c/releases/download/cpp-2.1.1/msgpack-2.1.1.tar.gz
tar -xzf msgpack-2.1.1.tar.gz
cd msgpack-2.1.1 && mkdir build && cd build
cmake -DMSGPACK_CXX11=ON -DMSGPACK_BUILD_EXAMPLES=OFF -DCMAKE_INSTALL_PREFIX=/usr ..
make -j4
make install
cd ../../

#install cereal
wget https://github.com/USCiLab/cereal/archive/v1.2.2.tar.gz
tar -xzf v1.2.2.tar.gz
cd cereal-1.2.2 && cd include
cp -R cereal /usr/local/include
cd ../../

git clone https://github.com/savoirfairelinux/opendht.git
cd opendht
mkdir build && cd build
cmake -DOPENDHT_PYTHON=ON -DCMAKE_INSTALL_PREFIX=/usr ..
make -j4
make install
cd ../../

git clone https://github.com/mutexunlocked/crypto
cd crypto
cd stanfordibe
cp /app/congenial-zephyr/src/Network/Server/docker/libssl.so.1.0.0 .
make
make install
cd ../../

git clone https://github.com/friedrich12/sodium-wrapper
cd sodium-wrapper
cd include
mkdir -p /usr/local/include/sodiumwrap
cp -R * /usr/local/include/sodiumwrap
cd ../../

git clone https://github.com/mutexunlocked/protobuf
cd protobuf
git submodule update --init --recursive
./autogen.sh
./configure
make
make install
ldconfig # refresh shared library cache.
cd ../

git clone https://github.com/mutexunlocked/grpc
cd grpc
git submodule update --init
make
make install
cd ../