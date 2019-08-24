LDFLAGS = -L/usr/local/lib -Wl,--no-as-needed -lgrpc++_reflection -lgrpc++ \
           -Wl,--as-needed -lprotobuf -lpthread -ldl -lcurl

ROOT_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

CXX = g++  -fomit-frame-pointer -fPIC
ROOT_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

CPPFLAGS += `pkg-config --cflags protobuf grpc`
CXXFLAGS += -std=c++11

GRPC_CPP_PLUGIN = grpc_cpp_plugin
GRPC_CPP_PLUGIN_PATH ?= `which $(GRPC_CPP_PLUGIN)`

all: client server

client: node.pb.o node.grpc.pb.o client.o
	$(CXX) $^ $(LDFLAGS) -o $@

server: node.pb.o node.grpc.pb.o server.o
	$(CXX) $^ $(LDFLAGS) -o $@

%.grpc.pb.cc: %.proto
	protoc --grpc_out=. --plugin=protoc-gen-grpc=$(GRPC_CPP_PLUGIN_PATH) $<

%.pb.cc: %.proto
	protoc --cpp_out=. $<

clean:
	rm -f *.o *.pb.cc *.pb.h client server
install:
	cp include/utils.hpp node.pb.h node.grpc.pb.h nodeserver.hpp nodeclient.hpp /usr/local/include/zephyr