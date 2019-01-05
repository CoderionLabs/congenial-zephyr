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
#include <sodium.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "shuffle.hpp"
#include <unistd.h>

#include <stdio.h>
#include <sys/types.h>//socket
#include <sys/socket.h>//socket
#include <string.h>//memset
#include <stdlib.h>//sizeof
#include <netinet/in.h>//INADDR_ANY

#include "mixerclient.h"
#include "mixerserver.hpp"
#include <jsonrpccpp/client/connectors/httpclient.h>
#include <jsonrpccpp/server/connectors/httpserver.h>


#define KEY_LENGTH  2048

#define PORT 8080
#define MAXSZ 4096

class Mixer
{
private:
    dht::DhtRunner node;
    std::vector<std::string> myip;
    std::vector<std::string> mixers;
    std::vector<std::string> mailboxes;
    std::string mixer_ip;
    unsigned char public_key[crypto_box_PUBLICKEYBYTES];
    unsigned char private_key[crypto_box_SECRETKEYBYTES];
    char id[20];
    int deadline;
    
public:
    Mixer(std::string mixerip, std::vector<std::string> mixers, std::vector<std::string> mailboxes);
    void ListenForMessages();
    void StartRoundAsCoordinator();
    void StartRoundAsMixer();
    ~Mixer();
};

void GetPrimaryIp(char* buffer, size_t buflen);
void ListenForMessages();

// Returns hostname for the local computer 
void checkHostName(int hostname) 
{ 
    if (hostname == -1) 
    { 
        perror("gethostname"); 
        exit(1); 
    } 
} 
  
// Returns host information corresponding to host name 
void checkHostEntry(struct hostent * hostentry) 
{ 
    if (hostentry == NULL) 
    { 
        perror("gethostbyname"); 
        exit(1); 
    } 
} 
  
// Converts space-delimited IPv4 addresses 
// to dotted-decimal format 
void checkIPbuffer(char *IPbuffer) 
{ 
    if (NULL == IPbuffer) 
    { 
        perror("inet_ntoa"); 
        exit(1); 
    } 
} 

