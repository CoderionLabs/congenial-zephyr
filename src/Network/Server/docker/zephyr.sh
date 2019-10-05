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

git clone https://github.com/friedrich12/sodium-wrapper
cd sodium-wrapper
cd include
mkdir -p /usr/local/include/sodiumwrap
cp -R * /usr/local/include/sodiumwrap
cd ../../

wget http://www.openssl.org/source/openssl-1.0.0a.tar.gz
tar -xf openssl-1.0.0a.tar.gz
cd openssl-1.0.0a
./config --prefix=/usr/local/openssl-1.0 shared
make
make install_sw
ls
cd ../
ls
cd congenial-zephyr
git checkout nightly
cd src
make
make install