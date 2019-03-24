# g++ main.cpp `pkg-config --cflags glib-2.0` -lzephyr
CC = g++

CPPFLAGS= -std=c++11 -fomit-frame-pointer -fPIC -DQHASM 
LDFLAGS= -shared -L/usr/local/lib -lcrypto -lssl  -lopendht -lgnutls -ljsoncpp -ljsonrpccpp-common -ljsonrpccpp-client

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