/*
 * Copyright (c) 2020 Mutex Unlocked
 * Author: Friedrich Doku
 * -----
 * Last Modified: Thursday April 18th 2020 9:41:47 pm
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


#include "pkgc.hpp"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
std::string key, params;

// Sends a WebSocket message and prints the response
std::vector<std::string> getkeysfrompkg(std::string hostname, std::string portnumber, std::string email)
{
    std::vector<std::string> myvec;
    try
    {

        auto const host = hostname;
        auto const port = portnumber;
        auto const text = email;

        // The io_context is required for all I/O
        net::io_context ioc;

        // These objects perform our I/O
        tcp::resolver resolver{ioc};
        websocket::stream<tcp::socket> ws{ioc};

        // Look up the domain name
        auto const results = resolver.resolve(host, port);

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
        ws.handshake(host, "/");

        // Send the email address
        ws.write(net::buffer(std::string(text)));

        beast::flat_buffer buffer;
        ws.read(buffer);

        // Close the WebSocket connection
        //ws.close(websocket::close_code::normal);


        // The make_printable() function helps print a ConstBufferSequence
        std::ostringstream os;
        os <<  beast::make_printable(buffer.data());
        std::string prompt = os.str();
        os.str("");
        os.clear();
        buffer.consume(buffer.size());

        std::string code;
        std::cout << prompt << std::endl;
        std::cin >> code;
        ws.write(net::buffer(std::string(code)));

        ws.read(buffer);
        os << beast::make_printable(buffer.data());
        std::string req = os.str();
        os.str("");
        os.clear();
        buffer.consume(buffer.size());
        std::cout << "REQ" << req << std::endl;
        if(req == "Looking good, I will send you your keys."){
            std::cout << "Getting Keys...";
            sleep(20);
            ws.read(buffer);
            os << beast::make_printable(buffer.data());
            std::string key = os.str();
            os.str("");
            os.clear();
            buffer.consume(buffer.size());

            ws.read(buffer);
            os << beast::make_printable(buffer.data());
            std::string params = os.str();
            os.str("");
            os.clear();
            buffer.consume(buffer.size());

            std::cout << "DONE" << std::endl;
            std::cout << "KEY " << key << std::endl;
            std::cout << "PARAMS " << params << std::endl;
            myvec.push_back(key);
            myvec.push_back(params);

            std::cout << "RECIVED " << sizeof(myvec[0]) + sizeof(myvec[1]) << " bytes" << std::endl;
            ws.close(websocket::close_code::normal);

        }else{
            std::cout << "FAILED" << std::endl;
            ws.close(websocket::close_code::abnormal);
        }
    }
    catch(std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return myvec;
    }
    return myvec;
}
