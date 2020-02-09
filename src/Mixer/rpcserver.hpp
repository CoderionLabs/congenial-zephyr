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

#pragma once

#include <rpc/server.h>
#include <string>
#include <iostream>
#include <vector>

class RpcServer
{
private:
    std::vector<std::string> messages;
public:
    void start();
    std::vector<std::string> getmessages();
};

std::string ping(){
    return "OKAY";
}

void RpcServer::start(){
    rpc::server srv(8000);

    srv.bind("ping", &ping);
    srv.bind("putmsg", [&](std::string msg){this->messages.push_back(msg); return "WORKS";});
    //srv.suppress_exceptions(true);
    srv.run();
}

std::vector<std::string> RpcServer::getmessages(){
    auto x = this->messages;
    this->messages.clear();
    return x;
}