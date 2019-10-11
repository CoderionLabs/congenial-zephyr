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

#pragma once

#include <iostream>
#include <map>
#include <string>
#include <algorithm>
#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/memory.hpp>

#include <fstream>
#include <vector>
#include <iostream>
#include <utility>
#include <map>

#include <cstdlib>

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/array.hpp>
#include <boost/asio.hpp>

#include <sodiumwrap/sodiumtester.h>
#include <sodiumwrap/box_seal.h>
#include <sodiumwrap/keypair.h>
#include <sodiumwrap/allocator.h>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
using bytes = sodium::bytes;
enum { max_length = 4096 };


struct publickeymap{
    std::map<std::string,std::string> pmap;
    
    template<class Archive>
    void serialize(Archive & archive)
    {
        archive(pmap);
    }
};
typedef struct publickeymap publickeymap;

struct keyvec{
    std::vector<unsigned char> data;

    template<class Archive>
    void serialize(Archive & archive)
    {
        archive(data); 
    }
};
typedef struct keyvec keyvec;


template <class T>
class Shuffle
{
public:
    void UnShuffle();
    Shuffle(std::vector<T> input, int seed);
    std::vector<T> vec;
private:
    int seed;
    int size;
};

// Fisher Yates Shuffle
template <class T>
Shuffle<T>::Shuffle(std::vector<T> input, int seed){
    std::copy(input.begin(), input.end(), std::back_inserter(this->vec));
    this->seed = seed;
    this->size = input.size();
    int random; srand(this->seed);

    for(int i = this->size -1; i > 0; i--){
        int index = rand() % (i + 1);
        std::swap(this->vec[index], this->vec[i]);
    }
}

// Fisher Yates De-shuffle
template <class T>
void Shuffle<T>::UnShuffle(){
    srand(this->seed);
    std::vector<T> randoms(this->size);
    int j = 0;
    for (int i = this->size - 1; i > 0; i--) {
        randoms[j++] = rand() % (i + 1);
    }

    //deShuffling
    for (int i = 1; i < this->size; i++) {
        //use the random values backwards
        int index = randoms[this->size - i - 1];
        // simple swap
        std::swap(this->vec[index], this->vec[i]);
    }
}

// Basic conversion functions needed for sending public keys
std::string ConvertMapToString(std::map<std::string,std::string> mymap);
std::map<std::string,std::string> ConvertStringToMap(std::string mapstring);
std::vector<std::vector<std::string>> get_config_info(std::string filename);
std::string talktonode(std::string ip, std::string port, std::string msg, bool recv);
std::pair<std::string, std::string> parseciphertext(std::string msg);
std::string serial_box_key(bytes k);
bytes deserial_box_key(std::string k);

inline std::string ConvertMapToString(std::map<std::string,std::string> mymap){
    publickeymap present{mymap};
    //Serialize
    std::stringstream ss;
    {
        // Create an output archive
        cereal::PortableBinaryOutputArchive oarchive(ss);

        oarchive(present); // Write the data to the archive
    }
    return ss.str();
}

inline std::pair<std::string, std::string> parseciphertext(std::string msg){
    std::string cut("CUTHERE");
    size_t found = msg.find("CUTHERE");
    if(found == std::string::npos){
        std::cerr << "Failed to parse argument" << std::endl;
        exit(1);
    }
    // cout << msg << endl;
    // cout << "Works 4" << endl;
    // cout << "FOUND " << found << endl;
    auto toread = msg.substr(found + cut.size());
    msg.erase(msg.begin() + found, msg.end());
    //toread.erase(toread.begin());

    // cout << toread << endl;
    // cout << msg << endl;
    // cout << "Works 5" << endl;

    int toread_start;
    std::istringstream iss (toread);
    iss >> toread_start;
    auto ip = msg.substr(msg.size() - toread_start);
    // cout << toread_start << endl;
    // cout << ip << endl;
    msg.erase(msg.end() - toread_start - toread.size(), msg.end());
    return std::make_pair(ip, msg);
}

inline std::map<std::string,std::string> ConvertStringToMap(std::string mapstring){
    std::stringstream ss;
    ss.write(mapstring.c_str(), mapstring.size());
    publickeymap c;
    {
        cereal::PortableBinaryInputArchive iarchive(ss);
        iarchive(c); // Read the data from the archive
    }
    return c.pmap;
}

inline std::string talktonode(std::string ip, std::string port, std::string msg, bool recv){
    try
    {
        boost::asio::io_context io_context;

        tcp::socket s(io_context);
        tcp::resolver resolver(io_context);
        boost::asio::connect(s, resolver.resolve(ip, port));

        boost::asio::write(s, boost::asio::buffer(msg, msg.size()));
        //sleep(10);
        if(recv){
            std::vector<char> buf(1024);
            size_t len = s.read_some(boost::asio::buffer(buf));
            std::string data(buf.begin(), buf.end());
            data.resize(len);
        
            return data;
        }
        return "";
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
}

inline std::string serial_box_key(bytes k){
    keyvec present{k};
    //Serialize
    std::stringstream ss;
    {
        // Create an output archive
        cereal::PortableBinaryOutputArchive oarchive(ss);

        oarchive(present); // Write the data to the archive
    }
    return ss.str();
}

inline bytes deserial_box_key(std::string k){
    std::stringstream ss;
    ss.write(k.c_str(), k.size());
    keyvec c;
    {
        cereal::PortableBinaryInputArchive iarchive(ss);
        iarchive(c); // Read the data from the archive
    }
    return c.data;
}


inline std::vector<std::vector<std::string>> get_config_info(std::string filename){
    std::ifstream in;
    in.open(filename);
    if(in.fail()){
        std::cout << "Failed to open file" << std::endl;
    }
    std::string data; std::map<std::string,int> types;
    std::vector<std::vector<std::string>> myvec(4);
    types["MIXERS"] = 1;
    types["MAILBOXES"] = 2;
    types["PKGS"] = 3;
    types["INFO"] = 4;
    int arr = 0;
    while(in >> data){
        //std::cout << data << std::endl;
        int tmp = types[data] -1;
        //std::cout << tmp << std::endl;
        if(tmp != -1){
            arr = tmp;
        }
        if (std::any_of(data.begin(), data.end(), ::isdigit)) {
            myvec[arr].push_back(data);
        }
    }
    std::cout << "DONE" << std::endl;
    return myvec;
}
//EXAMPLE FILE
// MIXERS
// 1.0.012
// 10.0.0.12
// 127.0.0.1
// MAILBOXES
// IP
// IP
// IP
// PKGS
// IP
// IP
// IP
// INFO
// IP
// IP