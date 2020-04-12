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

// Each client needs a stream socket representing its connection
// to S, a listen socket on which to accept incoming connections
// from peers, and at least two additional stream sockets with which
// to initiate outgoing connections to the other peer's public and private TCP endpoints.

// Client side C/C++ program to demonstrate Socket programming 
#include "common.hpp"


#define PORT 8080 

void GetPrimaryIp(char* buffer, size_t buflen) noexcept;
void connection_attempt(std::string ip);
void peer_server();
char privateip[16];

auto main(int argc, char const *argv[]) -> int
{ 
    GetPrimaryIp(privateip, 16); 

    std::cout << privateip << std::endl;

	int sock = 0, valread; 
	struct sockaddr_in serv_addr;
    struct sockaddr_in client_addr;

	char buffer[1024] = {0}; 
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{ 
		std::cout << "\n Socket creation error \n"; 
		return -1; 
	} 

	serv_addr.sin_family = AF_INET; 
	serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = inet_addr("142.93.196.152");

    client_addr.sin_family = AF_INET; 
	client_addr.sin_port = htons(PORT);

	// Convert IPv4 and IPv6 addresses from text to binary form 
	if(inet_pton(AF_INET, privateip, &client_addr.sin_addr)<=0) 
	{ 
		std::cout << "\nInvalid address/ Address not supported \n"; 
		return -1; 
	} 

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
	{ 
		std::cout << "\nConnection Failed \n"; 
		return -1; 
	} 
	send(sock , privateip , strlen(privateip) , 0 ); 
	std::cout << "privateip message sent\n"; 
	valread = read( sock , buffer, 1024); 
	std::cout << buffer << std::endl;

    bzero(buffer, sizeof(buffer));

    std::string tosend = "get" + std::string(privateip);
    send(sock , tosend.c_str() , tosend.length(), 0); 
	std::cout << "get message sent\n"; 
	valread = read( sock , buffer, 1024); 
	std::cout << buffer << std::endl;
    close(sock);

    auto peer_data = string_to_map(std::string(buffer));
    
    std::thread c1(connection_attempt, std::move(peer_data.begin()->first));
    std::thread c2(connection_attempt, std::move(peer_data.begin()->second.second));
    std::thread c3(peer_server);
    c1.detach();
    c2.detach();
    c3.detach();
    sleep(__INT_MAX__);
   
	return 0; 
}

void connection_attempt(std::string ip){
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
    serv_addr.sin_addr.s_addr = inet_addr(ip.c_str());
    client_addr.sin_family = AF_INET; 
	client_addr.sin_port = htons(PORT);

	if(inet_pton(AF_INET, privateip,
     &client_addr.sin_addr)<=0) std::cerr << "inet_pton failed" << std::endl; 
	bool gotit = false;
    while(!gotit){
        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
        { 
		    std::cout << "\nConnection Failed \n"; 
		    gotit = false; 
	    }else{
            gotit = true;
        }
    }

    std::string message;
	std::cout << "Connection Established " << std::endl;
    std::cout << "Enter Message: " << std::endl;
    std::cin >> message;

	send(sock , message.c_str() , message.size(), 0); 
	std::cout << "Message sent\n"; 
	valread = read(sock , buffer, 1024); 
	std::cout << buffer << std::endl;
    close(sock);

}

void peer_server(){
    int server_fd, new_socket, valread; 
	struct sockaddr_in address; 
	int opt = 1; 
	int addrlen = sizeof(address); 
	char buffer[1024] = {0}; 
	char *hello = "Hello from server"; 
	
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
	{ 
		std::cerr << "socket failed" << std::endl;
		exit(EXIT_FAILURE); 
	} 
	
    // The resuseaddr will be needed for the clients don't forget 
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
												&opt, sizeof(opt))) 
	{ 
		std::cerr << "setsockopt" << std::endl; 
		exit(EXIT_FAILURE); 
	} 
	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY; 
	address.sin_port = htons( PORT ); 
	
	// Forcefully attaching socket to the port 8080 
	if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) 
	{ 
		std::cerr << "bind failed" << std::endl;; 
		exit(EXIT_FAILURE); 
	} 
	if (listen(server_fd, 3) < 0) 
	{ 
		std::cerr << "listen" << std::endl; 
		exit(EXIT_FAILURE); 
	} 
	if ((new_socket = accept(server_fd, (struct sockaddr *)&address, 
					(socklen_t*)&addrlen))<0) 
	{ 
		std::cerr << "accept" << std::endl; 
		exit(EXIT_FAILURE); 
	}

	valread = read( new_socket , buffer, 1024); 
    std::cout << buffer << std::endl;
	send(new_socket , hello , strlen(hello) , 0 ); 
	std::cout << "Hello message sent\n" << std::endl; 
}
