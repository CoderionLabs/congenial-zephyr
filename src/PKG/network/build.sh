g++ -std=c++11 pkgserver.cpp -lboost_system -lsibe -lpthread -lpkg -o pkgs
g++ -std=c++11 pkgclient.cpp -lboost_system -lpthread -lpkg -o pkgc
