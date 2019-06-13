/*
 * Copyright (c) 2019 Doku Enterprise
 * Author: Friedrich Doku
 * -----
 * Last Modified: Sunday April 21st 2019 1:31:53 pm
 * -----
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


#include <zephyr/scan.hpp>
#include <zephyr/pkgc.hpp>
#include <zephyr/pkg.hpp>
#include <zephyr/Mixer.hpp>
#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sodium.h>

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>

extern "C"{
    #include <sibe/ibe.h>
    #include <sibe/ibe_progs.h>
}

CONF_CTX *cnfctx;
params_t params;
namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

using namespace std;
// Mixers, Mailboxes, PKGS

void error(const char *msg)
{
    perror(msg);
    exit(0);
}
std::string talktomixer(std::string ip, std::string msg);

int main(){
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
    cout << "PASSED HERE" << endl;
    cout << "Asking " << vec[2][1] << " for data" << endl;
    auto x = getkeysfrompkg(vec[2][1], to_string(8080), email);
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

    FILE * filePointer; 
    filePointer = fopen("params.txt","w+");
    params_out(filePointer, paramsb);
    fclose(filePointer);

    cout <<  "SIZE OF FULL DATA IS " << sizeof(keyb) + sizeof(paramsb) << endl;
    // Encrypt message for user
    cout << "MADE IT HERE 2" << endl;
    std::string encdata = pkg_encrypt("fried", paramsb, "LIfe is what we make of it.");
    cout << "MADE IT HERE FINISHED" << endl;
    // Select random mixer and send data to it
    int num = rand() % vec[0].size() -1;
    if(num == 0){
        num++;
    }
   
    string ip = vec[0][num];
    auto data = talktomixer(ip, "publickeys");
    auto map = ConvertStringToMap(data);
    auto mixenc = map[ip];
    
    cout << "START OF MIXENC" << endl;
    cout << mixenc << endl;
    cout << "END OF MIXENC" << endl;

    cout << "Talking to " << ip << endl;
    // Seal with crypto sercret box also append destination address to it data:ip
    int CIPHERTEXT_LEN = crypto_box_SEALBYTES + msg.length();
    unsigned char ciphertext[CIPHERTEXT_LEN];
    crypto_box_seal(ciphertext, reinterpret_cast<unsigned char*>(&msg[0]), msg.length(),
      reinterpret_cast<unsigned char*>(&mixenc[0]));
    // Send it back to the mixer
    auto gotbuf = talktomixer(ip, reinterpret_cast<char*>(ciphertext));
    cout << gotbuf << endl;
    return 0;
}

std::string talktomixer(std::string ip, std::string msg){
    try
    {
        // The io_context is required for all I/O
        net::io_context ioc;

        // These objects perform our I/O
        tcp::resolver resolver{ioc};
        websocket::stream<tcp::socket> ws{ioc};

        // Look up the domain name
        auto const results = resolver.resolve(ip, to_string(8080));

        // Make the connection on the IP address we get from a lookup
        net::connect(ws.next_layer(), results.begin(), results.end());

        // Set a decorator to change the User-Agent of the handshake
        ws.set_option(websocket::stream_base::decorator(
            [](websocket::request_type& req)
            {
                req.set(http::field::user_agent,
                    std::string(BOOST_BEAST_VERSION_STRING) +
                        " websocket-client-coro");
            }));

        // Perform the websocket handshake
        ws.handshake(ip, "/");

        // Send the message
        ws.write(net::buffer(std::string(msg)));

        beast::flat_buffer buffer;
        ws.read(buffer);

        // Close the WebSocket connection
        //ws.close(websocket::close_code::normal);


        // The make_printable() function helps print a ConstBufferSequence
        std::ostringstream os;
        os <<  beast::make_printable(buffer.data());
        std::string data = os.str();
        buffer.consume(buffer.size());
        ws.close(websocket::close_code::normal);
        return data;

    }
    catch(std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return "FAILED";
    }
}