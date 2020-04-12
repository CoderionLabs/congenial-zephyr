g++ helperserver.cpp -lpthread -o helper
g++ client.cpp  `pkg-config --cflags --libs glib-2.0` `pkg-config --cflags --libs gio-2.0` -lnice -o client