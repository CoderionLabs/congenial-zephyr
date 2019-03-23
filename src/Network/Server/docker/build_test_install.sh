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

wget http://www.openssl.org/source/openssl-1.0.0a.tar.gz
tar -xf openssl-1.0.0a.tar.gz
cd openssl-1.0.0a
./config --prefix=/usr/local/openssl-1.0
make
make install_sw
ls
cd ../
ls

cd Mixer
make
make install
cd ../
cd PKG
make
make install
cd ../
git clone https://github.com/DokuEnterprise/crypto
cd crypto
cd stanfordibe
cp ../../Network/Server/docker/libssl.so.1.0.0 .
make
make install


CMD ["/bin/bash"]