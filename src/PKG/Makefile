# g++ main.cpp `pkg-config --cflags glib-2.0` -lzepher
CC = g++

ROOT_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

CPPFLAGS= -std=c++11 -fomit-frame-pointer -fPIC -DQHASM  -I$(ROOT_DIR)/include
LDFLAGS= -shared -L/usr/local/lib -lsibe -lsodium -lboost_system -lcurl

SOURCES = $(shell echo *.cpp)
HEADERS = $(ROOT_DIR)/include/json.hpp $(ROOT_DIR)/include/pkg.hpp $(ROOT_DIR)/include/email.hpp $(ROOT_DIR)/include/pkgc.hpp $(ROOT_DIR)/include/base64.hpp $(ROOT_DIR)/include/pkgserial.hpp $(ROOT_DIR)/include/utf8.h 
UTF8HEADERS = $(ROOT_DIR)/include/utf8/checked.h $(ROOT_DIR)/include/utf8/core.h $(ROOT_DIR)/include/utf8/unchecked.h
OBJECTS=$(SOURCES:.cpp=.o)

TARGET=libpkg.so

all: $(TARGET)

.PHONY : clean
clean:
	rm -f $(OBJECTS)  $(TARGET)
	rm -f tester
	rm  -f $(ROOT_DIR)/network/pkgc
	rm  -f $(ROOT_DIR)/network/pkgs

$(TARGET) : $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)

install:
	mkdir -p /usr/local/include/zephyr
	mkdir -p /usr/local/include/zephyr/utf8
	cp $(HEADERS) /usr/local/include/zephyr
	cp $(UTF8HEADERS) /usr/local/include/zephyr/utf8
	cp libpkg.so /usr/local/lib

testfile:
	$(CC) -std=c++11 test/test.cpp -lpkg -lsibe -o tester
	./tester