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
