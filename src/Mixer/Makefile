# g++ main.cpp `pkg-config --cflags glib-2.0` -lzephyr
CC = g++

ROOT_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
 
#-ljsoncpp -ljsonrpccpp-common -ljsonrpccpp-client
CPPFLAGS= -std=c++14 -I$(ROOT_DIR)/../utils -fomit-frame-pointer -fPIC -DQHASM -lrpc
LDFLAGS= -shared -L/usr/local/lib  -lsodium  -lcrypto -lssl -lpthread -ljsonrpccpp-server -lopendht -lgnutls 

SOURCES = $(shell echo *.cpp)
HEADERS = $(shell echo *.hpp)
OBJECTS=$(SOURCES:.cpp=.o)

TARGET=libzephyr.so

all: $(TARGET)

.PHONY : clean
clean:
	rm -f $(OBJECTS)  $(TARGET)

$(TARGET) : $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)

install:
	cp $(HEADERS) /usr/local/include/zephyr
	cp libzephyr.so /usr/local/lib