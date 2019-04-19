# g++ main.cpp `pkg-config --cflags glib-2.0` -lzepher
CC = g++

ROOT_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

CPPFLAGS= -std=c++11 -fomit-frame-pointer -fPIC -DQHASM  -I=$(ROOT_DIR)/include
LDFLAGS= -shared -L/usr/local/lib -lsibe -lsodium -lboost_system -lcurl

SOURCES = $(shell echo *.cpp)
HEADERS = $(ROOT_DIR)/include/json.hpp $(ROOT_DIR)/include/pkg.hpp $(ROOT_DIR)/include/email.hpp $(ROOT_DIR)/include/pkgc.hpp
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
	cp $(HEADERS) /usr/local/include/zephyr
	cp libpkg.so /usr/local/lib

testfile:
	$(CC) -std=c++11 test/test.cpp -lpkg -o tester
	./tester