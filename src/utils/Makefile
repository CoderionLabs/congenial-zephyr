# g++ main.cpp `pkg-config --cflags glib-2.0` -lzepher
ROOT_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
HEADERS = $(ROOT_DIR)/include/utils.hpp 

install:
	mkdir -p /usr/local/include/zephyr
	cp $(HEADERS) /usr/local/include/zephyr