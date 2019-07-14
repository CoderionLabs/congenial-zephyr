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
#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <fstream>
#include <vector>
#include <iostream>
#include <map>

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/array.hpp>
#include <boost/asio.hpp>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
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

// Basic conversion functions needed for sending public keys
std::string ConvertMapToString(std::map<std::string,std::string> mymap);
std::map<std::string,std::string> ConvertStringToMap(std::string mapstring);
std::vector<std::vector<std::string>> get_config_info(std::string filename);
std::string talktonode(std::string ip, std::string port, std::string msg);


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

inline std::string talktonode(std::string ip, std::string port, std::string msg){
    try
    {
        boost::asio::io_context io_context;

        tcp::socket s(io_context);
        tcp::resolver resolver(io_context);
        boost::asio::connect(s, resolver.resolve(ip, port));

        boost::asio::write(s, boost::asio::buffer(msg, msg.size()));
        sleep(10);

        boost::asio::streambuf buffer;
        size_t reply_length = boost::asio::read(s,buffer);
        std::ostringstream out;
        
        out << beast::make_printable(buffer.data());
        std::string key = out.str();
        return std::string(key);
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
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
        if ((data.find("S") == std::string::npos) ||  (data.find("O") == std::string::npos)) {
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