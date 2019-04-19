#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <cstdlib>
#include <vector>
#include <iostream>
#include <string>

int getkeysfrompkg(std::string hostname, std::string portnumber, std::string email);