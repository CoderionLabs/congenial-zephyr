#pragma once

#include <iostream>
#include <vector>
#include "abstractmixerserver.h"

using namespace jsonrpc;
using namespace std;

class MixerServer : public AbstractMixerServer
{
private:
    std::vector<string> msgs;
    bool ismailman = false;
    bool nextnode = true;
public:
    MixerServer(AbstractServerConnector &conn, serverVersion_t type);

    virtual bool getMessage(const std::string& param1) override;
    virtual bool ping() override;
    virtual std::string request(const std::string& param1) override;
};

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
