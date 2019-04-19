#include <zephyr/scan.hpp>
#include <zephyr/pkgc.hpp>
#include <zephyr/pkg.hpp>
#include <zephyr/Mixer.hpp>
#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

using namespace std;
// Mixers, Mailboxes, PKGS

void error(const char *msg)
{
    perror(msg);
    exit(0);
}
std::string talktomixer(std::string ip, std::string msg);

int main(){
    string msg,email,key, params;
    cout << "Enter message" << endl;
    cin >> msg;
    cout << "Enter recievers email" << endl;
    cin >> email;

    // Load the configuration file
    string filepath = "";
    vector<vector<std::string>> vec;
    get_config_info(vec, filepath);
    getkeysfrompkg(vec[2][0], 55555, email);
    cout << "Now copy the KEY here" << endl;
    cin >> key;
    cout << "Now copy the PARAMS here" << endl;
    cin >> params;
    // Get your private key
    byte_string_t keyb;
    params_t paramsb;
    deserialize_bytestring(key, keyb);
    deserialize_params(params, paramsb);

    // Encrypt message for user
    pkg_encrypt(email, paramsb, msg);

    // Select random mixer and send data to it
    int num = rand() % vec[0].size() -1;
    string ip = vec[0][num];
    auto data = talktomixer(ip, "publickeys");
    auto map = ConvertStringToMap(data);
    auto mixenc = map[ip];
    // Seal with crypto sercret box also append destination address to it data:ip
    return 0;
}

std::string talktomixer(std::string ip, std::string msg){
    struct sockaddr_in address; 
    int sock = 0, valread; 
    struct sockaddr_in serv_addr; 

    bzero(buffer, sizeof(buffer));
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 
   
    memset(&serv_addr, '0', sizeof(serv_addr)); 
   
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(8080); 
       
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, ip, &serv_addr.sin_addr)<=0)  
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        return -1; 
    } 
   
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed \n"); 
        return -1; 
    } 
    send(sock , msg.c_str() , strlen(msg) , 0 ); 
    printf("Hello message sent\n"); 
    valread = read( sock , buffer, 1024); 
    return std::string(buffer);
}