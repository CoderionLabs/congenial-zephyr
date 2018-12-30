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
#include <fstream>
#include <thread>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "shuffle.hpp"
#include <unistd.h>

#include<stdio.h>
#include<sys/types.h>//socket
#include<sys/socket.h>//socket
#include<string.h>//memset
#include<stdlib.h>//sizeof
#include<netinet/in.h>//INADDR_ANY

#define PORT 8080
#define MAXSZ 4096

class PeerConnection
{
private:
    dht::DhtRunner node;
    std::vector<std::string> myip;
    std::vector<std::string> messages;
    std::vector<std::string> knownNodeAdresses;
    char id[20];
public:
    PeerConnection();
    void ListenForMessages();
    ~PeerConnection();
};

void GetPrimaryIp(char* buffer, size_t buflen);
void ListenForMessages();

