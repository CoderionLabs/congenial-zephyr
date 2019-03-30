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