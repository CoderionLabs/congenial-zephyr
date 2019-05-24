/*
 * Copyright (c) 2019 Doku Enterprise
 * Author: Friedrich Doku
 * -----
 * Last Modified: Friday May 24th 2019 10:36:03 am
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


#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <string>
#include <thread>

#include <zephyr/email.hpp>

#include <zephyr/pkg.hpp>
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


std::string serilized_parameters;
PKG p;

std::string from;
std::string from_password;
std::string email;
std::string code_tmp;
//------------------------------------------------------------------------------

void send_email(std::string email, std::string code){
    Email e;
	int curlError = 0;
	// e.dump();

	e.setTo(email);
	e.setFrom(from);
	e.setSubject("Zephyr Authentication");
	e.setCc("");
	e.setBody("Enter this " + code);

	e.setSMTP_host("smtps://smtp.gmail.com:465");
	e.setSMTP_username(from);
	e.setSMTP_password(from_password);

	//e.addAttachment("junk.txt");
	// e.addAttachment("email.h");
	// e.addAttachment("main.cpp");

	e.constructEmail();
	e.dump();

	curlError = e.send();

	if (curlError){
		std::cout << "Error sending email!" << std::endl;
	}

	else{
		std::cout << "Email sent successfully!" << std::endl;
	}
}


// Generate a random string to send to email
void gen_random(char *s, const int len) {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    for (int i = 0; i < len; ++i) {
        s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    s[len] = 0;
}

// Echoes back all received WebSocket messages
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
                        " websocket-server-sync");
            }));

        // Accept the websocket handshake
        ws.accept();

        std::ostringstream os;
        for(;;)
        {
            // This buffer will hold the incoming message
            beast::flat_buffer buffer;

            //  Read the email
            ws.read(buffer);
            os << boost::beast::make_printable(buffer.data());
            std::string email = os.str();
            os.str("");
            os.clear();
            buffer.consume(buffer.size());

            char code[24];
            gen_random(code,24);

            send_email(email, code);

            // Ask for the code
            ws.text(ws.got_binary());
            std::string msg = "I sent a code to your email. Please paste it in.";
            ws.write(net::buffer(std::string(msg)));
            os.str("");
            os.clear();
            buffer.consume(buffer.size());

            // Get the code and process
            ws.read(buffer);
            os << boost::beast::make_printable(buffer.data());
            std::string gotcode = os.str();
            os.str("");
            os.clear();
            buffer.consume(buffer.size());
            std::cout << "GOT CODE FROM CLIENT" << gotcode << std::endl;
            std::cout << "GOT CODE " << code << std::endl;
            if(gotcode == code){
                std::string msg = "Looking good, I will send you your keys.";
                ws.write(net::buffer(std::string(msg)));

                byte_string_t key;
                p.extract(email,key);
                std::string sendkey = p.serialize_bytestring(key);
                std::string sendparams = p.serialize_params();
                

                // Send the keys
                ws.write(net::buffer(std::string(sendkey)));
                ws.write(net::buffer(std::string(sendparams)));
                
            }else{
                std::string msg = "Wrong Code!";
                ws.write(net::buffer(std::string(msg)));
            }
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

int main(int argc, char* argv[])
{
    p.setup("dokuenterprise");
    try
    {
        // Check command line arguments.
         if (argc != 6)
    {
        std::cerr <<
            "Usage: pkgs <address> <port> <gmailuser> <gmailpassword>\n" <<
            "Example:\n" <<
            "    pkgs 0.0.0.0 8080 1 example@gmail.com 'password'\n" <<
            "    your password may need quotes if it has special characters";
        return EXIT_FAILURE;
    }
        auto const address = net::ip::make_address(argv[1]);
        auto const port = static_cast<unsigned short>(std::atoi(argv[2]));
        from = argv[4];
        from_password = argv[5];

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