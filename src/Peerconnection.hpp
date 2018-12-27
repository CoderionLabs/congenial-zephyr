#pragma once
#define MAX_BOOTSTRAP_NODES 20


#include <iostream>
#include <fstream>
#include <vector>

//Core Libs
#include <fstream>
#include <openssl/sha.h>
#include <opendht.h>
#include <vector>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>


class PeerConnection
{
private:
    dht::DhtRunner node;
    char myip[20];
    char id[20];
public:
    PeerConnection();
    ~PeerConnection();
};

void GetPrimaryIp(char* buffer, size_t buflen);

