#pragma once


#define MAX_BOOTSTRAP_NODES 20

#include <iostream>
#include <fstream>
#include <vector>

//Core Libs
#include <dht/dht.h>
#include <fstream>
#include <nice/agent.h>
#include <openssl/sha.h>
//#include <gio/gnetworking.h>

//DHT includes
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>
#include <sys/signal.h>

static volatile sig_atomic_t dumping = 0, searching = 0, exiting = 0;


static void
sigdump(int signo){ dumping = 1;}

static void
sigtest(int signo){searching = 1;}

static void
sigexit(int signo){exiting = 1;}



class Mixer
{
private:
    unsigned char myid[20];
    struct sockaddr_storage bootstrap_nodes[MAX_BOOTSTRAP_NODES];
    int num_bootstrap_nodes = 0;
    bool quiet = false;
    unsigned char buf[4096];
public:
    Mixer();
    ~Mixer();
};

static int set_nonblocking(int fd, int nonblocking);
static void init_signals(void);

