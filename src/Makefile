# g++ main.cpp `pkg-config --cflags glib-2.0` -lzephyr
CC = g++
MAKE = make
CPPFLAGS= -std=c++14 -fomit-frame-pointer -fPIC -DQHASM -lrpc
LDFLAGS= -shared -L/usr/local/lib -lcrypto -lssl -lsodium -lpthread -lopendht -lgnutls

SOURCES = $(shell echo *.cpp)
OBJECTS=$(shell echo Mixer/*.o && echo "utils/node.pb.o utils/node.grpc.pb.o ") 

ROOT_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
UTILHEADERS = $(shell echo utils/*.hpp)
HEADERS = $(shell echo Mixer/*.hpp)



TARGET=libzephyr.so

all: ready $(TARGET)

.PHONY : clean
clean:
	rm -f $(OBJECTS)  $(TARGET)

$(TARGET) : $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)

ready:
	cd Mixer && $(MAKE)
	cd utils && $(MAKE)
	cd PKG && $(MAKE) && $(MAKE) install

install:
	mkdir -p /usr/local/include/zephyr
	cp $(UTILHEADERS) /usr/local/include/zephyr
	cp $(HEADERS) /usr/local/include/zephyr
	cp libzephyr.so /usr/local/lib