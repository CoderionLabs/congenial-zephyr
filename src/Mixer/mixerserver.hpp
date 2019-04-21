/*
 * Copyright (c) 2019 Doku Enterprise
 * Author: Friedrich Doku
 * -----
 * Last Modified: Saturday March 30th 2019 5:44:39 pm
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


#pragma once

#include <iostream>
#include <vector>
#include "abstractmixerserver.hpp"

using namespace jsonrpc;
using namespace std;

class MixerServer : public AbstractMixerServer
{
private:
    bool ismailman = false;
    bool nextnode = true;
public:
    std::vector<string> msgs;
    MixerServer(AbstractServerConnector &conn, serverVersion_t type);

    virtual bool getMessage(const std::string& param1) override;
    virtual bool ping() override;
    virtual std::string request(const std::string& param1) override;
};