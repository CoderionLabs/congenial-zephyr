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

#include "common.hpp"

#define PORT 8080

std::ofstream out;
std::mutex mtx;           // mutex for critical section

auto main() -> int
{
    // This is what I will use to store peer data
    // TODO: The client will send me her private endpoint
    // I should already have her public endpoint
    // I will save this in my map
  
    int server_fd, new_socket, valread;
    struct sockaddr_in address;
    pid_t childpid;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[100000] = {0};
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
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        std::cerr << "bind failed" << std::endl;
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0)
    {
        std::cerr << "listen" << std::endl;
        exit(EXIT_FAILURE);
    }
    for (;;)
    {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                                 (socklen_t *)&addrlen)) < 0)
        {
            std::cerr << "accept" << std::endl;
            exit(EXIT_FAILURE);
        }

        // Let's get the client's public address
        std::cout << "PUBLIC IP OF CLIENT: " << inet_ntoa(address.sin_addr) << std::endl;
        if ((childpid = fork()) == 0)
        {
            close(server_fd);

            for (;;)
            {

                ReadXBytes(new_socket, buffer, 100000);
                std::cout << buffer << std::endl;
                std::string tmp_buf(buffer);
                if(tmp_buf.find("get") != std::string::npos){
                    auto mymap = file_to_map();
                    std::string rez = "";
                    std::string cmp = parse_get(tmp_buf);
                    while(rez == ""){
                        mymap = file_to_map();
                        for (auto x : mymap)
                        {
                            // std::cout << "CMP: " << cmp << std::endl;
                            // std::cout << "XFIRST: " << x.first << std::endl;
                            if (x.first != cmp)
                            {
                                //send the other peers public and private endpointsf
                                rez += x.second.first += std::string("CUTHERE") += x.second.second;
                                 for(int i = 0; i < rez.size(); i++){
                                    send(new_socket , reinterpret_cast<char*>(rez[i]), 1, 0); 
                                }
                                //send(new_socket, rez.c_str(), rez.length(), 0);
                                std::cout << "REZ " << rez << std::endl;
                                break;
                            }
                        }
                    }
                }else{
                    send(new_socket, hello, strlen(hello), 0);
                    std::cout << "Hello message sent\n"
                          << std::endl;
                    mtx.lock();
                    write_string_to_file(std::string(buffer) + "CUTHERE" + inet_ntoa(address.sin_addr));
                    bzero(buffer, strlen(buffer));
                    mtx.unlock();
                }
            }
        }
        
    }
    close(server_fd);

    return 0;
}
