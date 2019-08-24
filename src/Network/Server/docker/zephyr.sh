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