g++ server.cpp -lpthread -o helper
g++ peer.cpp  `pkg-config --cflags --libs glib-2.0` `pkg-config --cflags --libs gio-2.0` -lnice -o peer