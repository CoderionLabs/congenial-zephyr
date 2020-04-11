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

#include <unistd.h> 
#include <iostream>
#include <string>
#include <map>
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>

#define PORT 8080 

auto main() -> int
{ 
    // This is what I will use to store peer data
    // TODO: The client will send me her private endpoint
    // I should already have her public endpoint
    // I will save this in my map
    std::map<std::string, std::string> mymap;
    
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

    // Let's get the client's public address
    std::cout << "PUBLIC IP OF CLIENT: " << inet_ntoa(address.sin_addr) << std::endl;

	valread = read( new_socket , buffer, 1024); 
    std::cout << buffer << std::endl;
	send(new_socket , hello , strlen(hello) , 0 ); 
	std::cout << "Hello message sent\n" << std::endl; 
	return 0; 
} 
