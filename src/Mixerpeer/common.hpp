/*
 * Copyright (c) 2020 Mutex Unlocked
 * Author: Friedrich Doku
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *   This program is distributed in the hope that it will be useful
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <iostream>
#include <string.h>
#include <cassert>
#include <string>
#include <mutex> 
#include <array>
#include <map>
#include <netinet/in.h>
#include <fstream>
#include <thread> 
#define PORT 8080

auto write_string_to_file(std::string endpoints){
    std::ofstream out;
    out.open("data.txt", std::ios_base::app);
    if(out.fail()) return false;
    out << endpoints << std::endl;
    out.close();
    return true;
}

auto parse_get(std::string get){
    return get.erase(0,3);
}

auto file_to_map(){
    std::map<std::string, std::pair<std::string,std::string>> endpoints;

    std::ifstream infile("data.txt");
    std::string line;
    while(std::getline(infile,line)){
        std::string cut("CUTHERE");
        size_t found = line.find(cut);

        auto publicip = line.substr(found + cut.size());
        line.erase(line.begin() + found, line.end());

        endpoints[line] = std::make_pair(line, std::move(publicip));
    }
    return endpoints;
}

auto string_to_map(std::string str){
    std::map<std::string, std::pair<std::string,std::string>> endpoints;
   
    std::string cut("CUTHERE");
    size_t found = str.find(cut);

    auto publicip = str.substr(found + cut.size());
    str.erase(str.begin() + found, str.end());

    endpoints[str] = std::make_pair(str, std::move(publicip));
    
    return endpoints;
}

// Gets the private ip address of the client
auto GetPrimaryIp(char* buffer, size_t buflen)
{
    assert(buflen >= 16);

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    assert(sock != -1);

    const char* kGoogleDnsIp = "8.8.8.8";
    uint16_t kDnsPort = 53;
    struct sockaddr_in serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = inet_addr(kGoogleDnsIp);
    serv.sin_port = htons(kDnsPort);

    int err = connect(sock, (const sockaddr*) &serv, sizeof(serv));
    assert(err != -1);

    sockaddr_in name;
    socklen_t namelen = sizeof(name);
    err = getsockname(sock, (sockaddr*) &name, &namelen);
    assert(err != -1);

    const char* p = inet_ntop(AF_INET, &name.sin_addr, buffer, buflen);
    assert(p);
    close(sock);
}

auto send_connection_string(std::string conn_str){
    char privateip[16];

    GetPrimaryIp(privateip, 16); 

    std::cout << privateip << std::endl;

	int sock = 0, valread; 
	struct sockaddr_in serv_addr;
    struct sockaddr_in client_addr;

	char buffer[1024] = {0}; 
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{ 
		std::cout << "\n Socket creation error \n"; 
	} 

	serv_addr.sin_family = AF_INET; 
	serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    client_addr.sin_family = AF_INET; 
	client_addr.sin_port = htons(PORT);

	// Convert IPv4 and IPv6 addresses from text to binary form 
	if(inet_pton(AF_INET, privateip, &client_addr.sin_addr)<=0) 
	{ 
		std::cout << "\nInvalid address/ Address not supported \n"; 
	} 

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
	{ 
		std::cout << "\nConnection Failed \n"; 
	} 
	send(sock , conn_str.c_str() , conn_str.size(), 0 ); 
	std::cout << "Connection string message sent\n"; 
	valread = read( sock , buffer, 1024); 
	std::cout << buffer << std::endl;

    bzero(buffer, sizeof(buffer));

    std::string tosend = "get" + conn_str;
    send(sock , tosend.c_str() , tosend.length(), 0); 
	std::cout << "get message sent\n"; 
	valread = read( sock , buffer, 1024); 
	std::cout << buffer << std::endl;
    close(sock);

    return string_to_map(std::string(buffer));
}