/*
 * Copyright (c) 2019 Doku Enterprise
 * Author: Friedrich Doku
 * -----
 * Last Modified: Saturday March 30th 2019 5:44:29 pm
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


#include <iostream>
#include <vector>
#include "mixerserver.hpp"

MixerServer::MixerServer(AbstractServerConnector &connector,
                           serverVersion_t type)
    : AbstractMixerServer(connector, type) {}

bool MixerServer::getMessage(const std::string& param1){
    this->msgs.push_back(param1);
    return true;
}

std::string MixerServer::request(const std::string& param1){
    // TODO: parse requets
    if(param1 == "mailman"){
        // A mixer wants to be the mailman
        if(!ismailman){
            return "mailmangranted";
        }
    }
    if(param1 == "messages"){
        this->msgs.erase(this->msgs.begin());
        if(!ismailman){
            if(this->msgs.empty()){
                return "0";
            }
            return this->msgs[0];
        }
    }
    if(param1 == "nextnode"){
        if(nextnode){
            nextnode = false;
            return "yes";
        }else{
            return "no";
        }
    }
}

bool MixerServer::ping(){
    return true;
}
