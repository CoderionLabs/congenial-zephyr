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

void
do_session(tcp::socket& socket)
{
    try
    {
        // Construct the stream by moving in the socket
        websocket::stream<tcp::socket> ws{std::move(socket)};

        // Set a decorator to change the Server of the handshake
        ws.set_option(websocket::stream_base::decorator(
            [](websocket::response_type& res)
            {
                res.set(http::field::server,
                    std::string(BOOST_BEAST_VERSION_STRING) +
                        " server");
            }));

        // Accept the websocket handshake
        ws.accept();
    
        std::string client_public_address = socket.remote_endpoint().address().to_string();


        std::ostringstream os;
        for(;;)
        {
            // This buffer will hold the incoming message
            beast::flat_buffer buffer;

            // Read a message
            ws.read(buffer);
            os << boost::beast::make_printable(buffer.data());
            std::string data = os.str();
            os.str("");
            os.clear();
            buffer.consume(buffer.size());

            if(data.find("get") != std::string::npos){
                    auto mymap = file_to_map();
                    std::string rez = "";
                    std::string cmp = parse_get(data);
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
                                 
                                ws.text(false);
                                ws.write(net::buffer(rez));
                                std::cout << "REZ " << rez << std::endl;
                                break;
                            }
                        }
                    }
                }else{
                    ws.text(false);
                    ws.write(net::buffer("Hello"));
                    std::cout << "Hello message sent\n"
                          << std::endl;
                    mtx.lock();
                    write_string_to_file(data + "CUTHERE" + client_public_address);
                    buffer.consume(buffer.size());
                    mtx.unlock();
                }


            // Echo the message back
            
        }
    }
    catch(beast::system_error const& se)
    {
        // This indicates that the session was closed
        if(se.code() != websocket::error::closed)
            std::cerr << "Error: " << se.code().message() << std::endl;
    }
    catch(std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

//------------------------------------------------------------------------------

auto main(int argc, char* argv[]) -> int
{
    try
    {
        // Check command line arguments.
        if (argc != 2)
        {
            std::cerr <<
                "Usage: server <address>\n" <<
                "Example:\n" <<
                "    server 0.0.0.0\n";
            return EXIT_FAILURE;
        }
        auto const address = net::ip::make_address(argv[1]);
        auto const port = static_cast<unsigned short>(PORT);

        // The io_context is required for all I/O
        net::io_context ioc{1};

        // The acceptor receives incoming connections
        tcp::acceptor acceptor{ioc, {address, port}};
        for(;;)
        {
            // This will receive the new connection
            tcp::socket socket{ioc};

            // Block until we get a connection
            acceptor.accept(socket);
        
            // Launch the session, transferring ownership of the socket
            std::thread{std::bind(
                &do_session,
                std::move(socket))}.detach();
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}

