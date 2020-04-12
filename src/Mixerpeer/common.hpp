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

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/memory.hpp>

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <string>
#include <thread>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

#define PORT 8080


struct condata{
    std::string conn_str;
    
    template<class Archive>
    void serialize(Archive & archive)
    {
        archive(conn_str);
    }
};
typedef struct condata condata;

std::string condata_to_string(std::string con_str){
    condata present{con_str};
    //Serialize
    std::stringstream ss;
    {
        // Create an output archive
        cereal::PortableBinaryOutputArchive oarchive(ss);

        oarchive(present); // Write the data to the archive
    }
    return ss.str();
}


std::string condata_from_string(std::string con_str_serial){
    std::stringstream ss;
    ss.write(con_str_serial.c_str(), con_str_serial.size());
    condata c;
    {
        // Create an output archive
        cereal::PortableBinaryInputArchive iarchive(ss);

        iarchive(c); // Write the data to the archive
    }
    return c.conn_str;
}

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

void ReadXBytes(int socket,  char* buffer, unsigned int x)
{
    int bytesRead = 0;
    int result;
    while (bytesRead < x)
    {
        result = read(socket, buffer + bytesRead, x - bytesRead);
        std::cout << std::string(buffer) << std::endl;
        if (result < 1 )
        {
            std::cerr << "FAILED TO READ " << std::endl;
        }

        bytesRead += result;
    }
}

auto send_connection_string(std::string conn_str){

    std::cout << "I'M IN HERE" << std::endl;
    char privateip[16];
    GetPrimaryIp(privateip, 16); 
    try
    {
        std::string host = "142.93.196.152";
        auto const  port = PORT;
        auto const  text = conn_str;
        beast::flat_buffer buffer;
        std::ostringstream os;

        net::io_context ioc;
        tcp::resolver resolver{ioc};
        websocket::stream<tcp::socket> ws{ioc};

        auto const results = resolver.resolve(host, std::to_string(port));

        // Make the connection on the IP address we get from a lookup
        auto ep = net::connect(ws.next_layer(), results);
        host += ':' + std::to_string(ep.port());

        ws.set_option(websocket::stream_base::decorator(
            [](websocket::request_type& req)
            {
                req.set(http::field::user_agent,
                    std::string(BOOST_BEAST_VERSION_STRING) +
                        " websocket-client-coro");
            }));
        ws.handshake(host, "/");
       // ws.text(false);
        ws.binary(true);


        std::cout << "HERE 2" << std::endl;
        ws.write(net::buffer(std::string(condata_to_string(conn_str))));

        std::cout << "HERE X" << std::endl;

        ws.read(buffer);
        std::cout << "HERE Y" << std::endl;

        os << boost::beast::make_printable(buffer.data());
        std::string data = condata_from_string(os.str());
        std::cout << data << std::endl;

        std::cout << "HERE 3" << std::endl;

        os.str("");
        os.clear();
        buffer.consume(buffer.size());
        data.clear();

        ws.write(net::buffer(condata_to_string("get"+conn_str)));

        ws.read(buffer);
        os << boost::beast::make_printable(buffer.data());
        data = condata_from_string(os.str());
        std::cout << "REMOTE DATA" << std::endl;
        std::cout << data << std::endl;
        std::cout << "REMOTE DATA END" << std::endl;

        os.str("");
        os.clear();
        buffer.consume(buffer.size());

        // Close the WebSocket connection
        ws.close(websocket::close_code::normal);
            std::cout << "I'M OUT" << std::endl;
        return string_to_map(std::string(data));
    }
    catch(std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
        std::cout << "I'M OUT" << std::endl;

    return  string_to_map("");;
}