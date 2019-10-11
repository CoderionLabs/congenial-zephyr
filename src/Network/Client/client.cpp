/*
 * Copyright (c) 2019 Doku Enterprise
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


#include <zephyr/pkgc.hpp>
#include <zephyr/pkg.hpp>
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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <thread>
#include <netdb.h>
#include <sodium.h>
#include <sodiumwrap/sodiumtester.h>
#include <sodiumwrap/box_seal.h>
#include <sodiumwrap/keypair.h>
#include <sodiumwrap/allocator.h>
extern "C"{
    #include <sibe/ibe.h>
    #include <sibe/ibe_progs.h>
}

CONF_CTX *cnfctx;
params_t params;
vector<vector<std::string>> config;
std::vector<std::string> msgtmp;

using namespace std;
using namespace node;
using namespace sodium;


// Mixers, Mailboxes, PKGS

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

void RunServerInBackground();
std::vector<std::string> getkeysfrominfo();
std::string attachtomixer(std::string msg);
int createciphertext(std::map<std::string,std::string> mixerKeys, std::string encmsg);

int main(){

    sodium_init();

    std::thread t1(RunServerInBackground);
    t1.detach();
    
    string msg,email,key, params, filepath;
    cout << "Enter message" << endl;
    cin >> msg;
    cout << "Enter recievers email" << endl;
    cin >> email;
    cout << "Enter config file path" << endl;
    cin >> filepath;

    // Load the configuration file
    //string filepath = "";
    vector<vector<std::string>> vec;
    vec = get_config_info(filepath);
    config = vec;
    cout << "PASSED HERE" << endl;
    cout << "Asking " << vec[2][0] << " for data" << endl;
    auto x = getkeysfrompkg(vec[2][0], to_string(8080), email);
    cout << "GOT IT" << endl;
    // Get your private key
    byte_string_t keyb;
    params_t paramsb;
    IBE_init();
    cout << x[0] << endl;
    cout << x[1] << endl;
    std::string key_serial_tmp = std::move(x[0]);
    std::string param_serial_tmp = std::move(x[1]);
    deserialize_bytestring(key_serial_tmp, keyb);
    deserialize_params(param_serial_tmp, paramsb);

    // FILE * filePointer; 
    // filePointer = fopen("params.txt","w+");
    // params_out(filePointer, paramsb);
    // fclose(filePointer);

    cout << "SIZE OF FULL DATA IS " << sizeof(keyb) + sizeof(paramsb) << endl;
    // Encrypt message for user
    cout << "MADE IT HERE 2" << endl;

    std::string encdata = pkg_encrypt("fried", paramsb, "LIfe is what we make of it.");
    cout << "MADE IT HERE FINISHED" << endl;
    
    // Get mixer data from information node
    getkeysfrominfo();
    for(auto x : msgtmp){
        cout << x << endl;
    }
    // cout << "CONVERTED !!!" << endl;
    // createciphertext(mixerKeys, encdata);

    return 0;
}

int createciphertext(std::map<std::string,std::string> mixerKeys, std::string encmsg){

    std::vector<std::string> mixers;
    for(auto x : mixerKeys){
        // Push all the IPs to a vector
        mixers.push_back(x.first);
        // cout << x.first << endl;
        // cout << x.second << endl;
    }
    cout << "Works 1" << endl;

    std::mt19937 rng;
    rng.seed(std::random_device()());
    std::uniform_int_distribution<std::mt19937::result_type> dist6(1,10);
    auto seed = dist6(rng);

    Shuffle<std::string> shu(mixers, (int) seed);
    mixers.clear();
    mixers = std::move(shu.vec);
    cout << "Works 2" << endl;

    int N = mixers.size();

    box_seal<> sb{};
    std::vector<sodium::keypair<>> boxes;
    for(int i = 0; i < N; i++){
        auto str = mixerKeys[mixers[i]];
        bytes mixkey = deserial_box_key(str);
        sodium::keypair<> mix{};
        mix.public_key_ = mixkey;

        boxes.push_back(mix);
    }
    
    //TODO: Create mailbox code and fix addresses
    std::string mailboxaddress = "NULL";

    std::string enctmp = encmsg;
    enctmp += mailboxaddress;
    enctmp += std::to_string(mailboxaddress.size());

    cout << "Done Setting getting mixer keys and adding mailbox." << endl;
    
    bytes tmpenc{enctmp.cbegin(), enctmp.cend()};
    int i = 0;
    for(auto x : boxes){
        tmpenc =  sb.encrypt(tmpenc, x.public_key());

        std::string cutgo;
        cutgo += mixers[i];   // Address
        cutgo += "CUTHERE";
        cutgo += to_string(mixers[i].size()); // Size of ip address

        std::string tt{tmpenc.cbegin(), tmpenc.cend()};
        tt +=cutgo;

        bytes tempenctmp{tt.cbegin(), tt.cend()};
        tmpenc = tempenctmp;
        i++;
    }

    std::string exitenc{tmpenc.cbegin(), tmpenc.cend()};

    attachtomixer(exitenc);
    return 0;
}

std::string attachtomixer(std::string msg){

    string cut("CUTHERE");
    size_t found = msg.find("CUTHERE");
    if(found == std::string::npos){
        cout << "Failed to parse argument" << endl;
        exit(1);
    }
    cout << msg << endl;
    cout << "Works 4" << endl;
    cout << "FOUND " << found << endl;
    auto toread = msg.substr(found + cut.size());
    msg.erase(msg.begin() + found, msg.end());
    //toread.erase(toread.begin());

    cout << toread << endl;
    cout << msg << endl;
    cout << "Works 5" << endl;

    int toread_start;
    std::istringstream iss (toread);
    iss >> toread_start;
    auto ip = msg.substr(msg.size() - toread_start);
    cout << toread_start << endl;
    cout << ip << endl;
    msg.erase(msg.end() - toread_start - toread.size(), msg.end());
    cout << msg << endl;
    cout << "Works 6" << endl; 
    

    NodeClient mixreq(
    grpc::CreateChannel(ip + ":50051",
                          grpc::InsecureChannelCredentials()));

    std::cout << "-------------- GetMessages --------------" << std::endl;
    node::Msg tosend;
    tosend.set_data(msg);
    mixreq.data.push_back(tosend);
    mixreq.PutMessages();

    return "";
}

void getkeysfrominfo(){

    int num = rand() % config[3].size() -1;
    auto x = config[3][num];
    std::cout << "Getting keys from " << x << std::endl;

    NodeClient mixreq(
    grpc::CreateChannel(config[3][num] + ":50051",
                          grpc::InsecureChannelCredentials()));

    std::cout << "-------------- GetMessages --------------" << std::endl;
    node::Msg tosend;
    tosend.set_data("NEED127.0.0.1");
    mixreq.data.push_back(tosend);
    mixreq.PutMessages();
}

void RunServerInBackground(){
    std::string server_address(INFONODEIP + ":50051");
    NodeImpl service;

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;
    server->Wait();
}