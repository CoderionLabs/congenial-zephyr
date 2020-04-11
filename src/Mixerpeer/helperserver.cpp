#include <unistd.h> 
#include <iostream>
#include <string>
#include <map>
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h>

#define PORT 8080 

int main() 
{ 
    // This is what I will use to store peer data
    // TODO: The client will send me her private endpoint
    // I should already have her public endpoint
    // I will save this in my map
    std::map<string, string> mymap;
    
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
	if (bind(server_fd, (struct sockaddr *)&address, 
								sizeof(address))<0) 
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
	return 0; 
} 
