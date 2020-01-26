/*
 * Copyright (c) 2020 Doku Enterprise
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


#include <zephyr/nodeclient.hpp>
#include <zephyr/nodeserver.hpp>
#include <zephyr/node.grpc.pb.h>
#include <zephyr/utils.hpp>
#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <thread>


std::vector<std::string> outbox;
std::vector<std::string> msgtmp;

std::string MAILNODEIP;
void RunServerInBackground();


int main(int argc, char* argv[]){
    
    if (argc != 2){
        std::cerr << "Usage: mailbox MAILNODEIP\n";
        return 1;
    }
    MAILNODEIP = argv[1];

    std::thread t1(RunServerInBackground);
    t1.detach();

    while(true){
        if(!msgtmp.empty()){
            for(auto x : msgtmp){
                outbox.push_back(x);
            }
            msgtmp.clear();
        }
    }
    

    return 0;
}


void RunServerInBackground(){
    std::string server_address(MAILNODEIP + ":50051");
    NodeImpl service;

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;
    server->Wait();
}