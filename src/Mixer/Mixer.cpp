/*
 * Copyright (c) 2019 Doku Enterprise
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


#include "Mixer.hpp"

using namespace std;
using namespace node;
using namespace sodium;
//GLOBALS...
std::string MIXERIP;
std::vector<std::string> ipspub;
std::vector<std::string> reqtmp;
bool chooseinfo = false;

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

void Mixer::CleanUp(){

    std::condition_variable cv;
    std::mutex m;
    bool done {false};

    this->node.shutdown([&]()
    {
        std::lock_guard<std::mutex> lk(m);
        done = true;
        cv.notify_all();
    });

    // wait for shutdown
    std::unique_lock<std::mutex> lk(m);
    cv.wait(lk);
}


void Mixer::Start(std::string mixerip, std::vector<std::string> mixers,
 std::vector<std::string> mailboxes, std::string configpath){

     // Initilize variables
    this->mixerip = mixerip;
    this->mailboxes = mailboxes;
    this->mixers = mixers;
    this->configpath = configpath;

    auto config = get_config_info(this->configpath);
    if(config[0][0] == this->mixerip){
        chooseinfo = true;
    }
    //dht::DhtRunner node;
    MIXERIP = this->mixerip;
    auto mtx = std::make_shared<std::mutex>();
    auto cv = std::make_shared<std::condition_variable>();
    auto ready = std::make_shared<bool>(false);

    std::cout << "Creating Keys..." << std::endl;


    auto tmpid = this->node.getNodeId().to_c_str();
    std::cout << "DONE" << std::endl;
    
    memcpy(this->id, tmpid, sizeof(tmpid));    

    // listen on port 4222.
    std::cout << "Starting DHT this->node..." << std::endl;
    this->node.run(4222, dht::crypto::generateIdentity(), true);
    std::cout << "DONE" << std::endl;

    // The first node in the network will not use a bootstrap
    // node to join the network. Mixer address will be loaded from the 
    // config file. The first mixer in the list will be used as a bootstrap node
    std::cout << "BOOTSTRAPING..." << std::endl;
    this->node.bootstrap("bootstrap.ring.cx", "4222");
    std::cout << "DONE" << std::endl;

    auto wait = [=] {
        *ready = true;
        std::unique_lock<std::mutex> lk(*mtx);
        cv->wait(lk);
        *ready = false;
    };
    auto done_cb = [=](bool success) {
        if (success) {
            std::cout << "success!" << std::endl;
        } else {
            std::cout << "failed..." << std::endl;
        }
        std::unique_lock<std::mutex> lk(*mtx);
        cv->wait(lk, [=]{ return *ready; });
        cv->notify_one();
    };

    string plugin; string r = "ready";
    auto tmpstrkey = serial_box_key(this->mix.public_key());
    plugin = string(tmpstrkey + "_____________________________________________" + mixerip);

    this->node.put("publickeys", dht::Value((const uint8_t*)plugin.data(), plugin.size()), [=] (bool success) {
        std::cout << "Put public key: ";
        done_cb(success);   
    });

    // block to see the result of the put
    wait();

    this->node.put("ready",dht::Value((const uint8_t*) r.data(), r.size()), [=] (bool success){
        std::cout << "Put read: ";
        done_cb(success);
    });

    // block to see the result of the put
    wait();
    //this->node.getPeerDiscovery();

    // Wait for all the other nodes to be ready
    std::cout << "Waiting for other nodes" << std::endl;
    while(true){
        this->node.get(
            "ready",
            [&](const std::vector<std::shared_ptr<dht::Value>>& values) {
                for (const auto& v : values){
                    this->readymixers++;
                    std::string power {v->data.begin(), v->data.end()};
                    std::cout << "Found value: " << power << std::endl;
                }
             return true; // keep looking for values
            },
            [=](bool success) {
                std::cout << "That's all I found" << std::endl;
                std::cout << "Getting mixers ready: " << (success ? "success" : "failure") << std::endl;
                done_cb(success);
            }
        );
        wait();

        if(this->readymixers == mixers.size()){
            break;
        }else{
            std::cout << "Not everyone is ready yet sleeping " << mixers.size() << " needed" << std::endl;
            for(auto x : mixers){
                cout << x << endl;
            }
            std::cout << this->readymixers << std::endl;
            sleep(10);
            this->readymixers = 0;
        }
    }

    //Get the public keys still work to do 
    this->node.get(
        "publickeys",
            [](const std::vector<std::shared_ptr<dht::Value>>& values) {
                for (const auto& v : values){
                    std::string mydata {v->data.begin(), v->data.end()};
                    std::cout << "FOUND: " << mydata << std::endl;
                    
                    size_t pos = 0;
                    string token = "_____________________________________________";
                    pos = mydata.find(token);
                    std::cout << pos << std::endl;
                    string pub = mydata.substr(0,pos);
                    std::cout << "PUBLIC KEY: " <<  pub << std::endl;
                    string ip = mydata.erase(0, pos + token.length());
                    std::cout << "IP: " << ip << std::endl;

                    GiveMeDataForPublic(mydata);
                }
                return true; // keep looking for values
            },
            [=](bool success) {
                std::cout << "Getting mixers ready: " << (success ? "success" : "failure") << std::endl;
                done_cb(success);
            }
    );

    wait();

    std::cout << "THESE ARE THE KEYS I HAVE START" <<  std::endl;
    for(auto x : ipspub){
         std::cout << x <<  std::endl;
    }
    std::cout << "THESE ARE THE KEYS I HAVE END" <<  std::endl;

    //this->node.join();

    vector<vector<std::string>> vec;
    vec = get_config_info(configpath);

    // Give pulickeys to the random infonode
    // the infonode will then distribute
    // the message to all other infonodes

    if(chooseinfo){
        int num = rand() % config[3].size() -1;
        auto x = config[3][num];
        std::cout << "Talking to " << x << std::endl;
        NodeClient toinfo(
        grpc::CreateChannel(x + ":50051",
                          grpc::InsecureChannelCredentials()));

        std::cout << "-------------- SENDING MESSAGES TO INFO NODE --------------" << std::endl;
        for(auto x : ipspub){
            Msg msg;
            msg.set_data(x);
            toinfo.data.push_back(msg);
        }
       

        toinfo.PutMessages();
        
    }
    this->StartRoundAsMixer();
}

void GiveMeDataForPublic(std::string data){
    ipspub.push_back(data);
}

Mixer::~Mixer(){
    // Before leaving add the list of known nodes to a
    // file and also put them in the DHT
    ofstream out("nodes.dems");
    for(auto x : this->mixers){
        out << x << endl;
    }
    out.close();

    this->node.putSigned("ROUTING_TABLE:" + string(this->id), this->mixers, [](bool ok){
        if(not ok){
            cout << "Failed to publish known nodes" << endl;
        }
    });
    // Shutdown the DHT node
    this->CleanUp();
}

void RunServerInBackground(){
    std::string server_address(MIXERIP + ":50051");
    NodeImpl service;

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;
    server->Wait();
}


void Mixer::StartRoundAsMixer(){
    //Start a server in the background
    std::thread runserver(RunServerInBackground);
    runserver.detach();


    while(true){
        if(!msgtmp.empty()){
            std::cout << "THIS WORKS" << std::endl;
            std::copy(msgtmp.begin(), msgtmp.end(), std::back_inserter(reqtmp));
            msgtmp.clear();
            std::cout << "IM IN HERE " << reqtmp.size() << std::endl;
            if(reqtmp.size() != 0){
                std::mt19937 rng;
                rng.seed(std::random_device()());
                std::uniform_int_distribution<std::mt19937::result_type> dist6(1,10);

                auto x = dist6(rng);
                std::cout << "THIS IS IT" << std::endl;
                std::cout << reqtmp[0] << std::endl;

                Shuffle<std::string> shu(reqtmp, (int) x);
                reqtmp.clear();
                reqtmp = std::move(shu.vec);

                // Strip off a layer of encryption and send to the next
                // mixer.
                for(auto x : reqtmp){
                    //unsigned char decrypted[1000];
                    box_seal<> sb{};
                    bytes enc{ x.cbegin(), x.cend() };
                    auto decrypted = sb.decrypt(enc, this->mix.private_key(), this->mix.public_key());
                    // crypto_box_seal_open(decrypted, reinterpret_cast<const unsigned char*>(x.c_str()),
                    // x.length(), this->public_key, this->private_key);

                    //FIXME: Fix this
                    std::string conv{decrypted.cbegin(), decrypted.cend()};
                    std::cout << "THIS IS THE MESSAGE I GOT " << conv << std::endl;               

                    auto p = parseciphertext(conv);
                    std::string ip = p.first;
                    std::string msg = p.second;
                    std::cout << "THE NEXT MIXER WILL BE " <<  ip << std::endl;
                    senddata(ip, msg);
                }
                reqtmp.clear();
            }
        }
    }
}

// Send data to the next node
void senddata(std::string ip, std::string msg){
   NodeClient guide(
      grpc::CreateChannel(ip + ":50051",
                          grpc::InsecureChannelCredentials()));

  std::cout << "-------------- GetMessages --------------" << std::endl;
  Msg x;
  x.set_data(msg);
  guide.data.push_back(x);
  guide.PutMessages();
}

// Returns hostname for the local computer 
void checkHostName(int hostname) 
{ 
    if (hostname == -1) 
    { 
        perror("gethostname"); 
        exit(1); 
    } 
} 
  
// Returns host information corresponding to host name 
void checkHostEntry(struct hostent * hostentry) 
{ 
    if (hostentry == NULL) 
    { 
        perror("gethostbyname"); 
        exit(1); 
    } 
} 

// Converts space-delimited IPv4 addresses 
// to dotted-decimal format 
void checkIPbuffer(char *IPbuffer) 
{ 
    if (NULL == IPbuffer) 
    { 
        perror("inet_ntoa"); 
        exit(1); 
    } 
} 

// Gets the global ip address of the server
void GetPrimaryIp(char* buffer, size_t buflen) 
{
    assert(buflen >= 16);

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    assert(sock != -1);

    const char* kGoogleDnsIp = "8.8.8.8";
    uint16_t kDnsPort = 53;
    struct sockaddr_in serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = inet_addr(kGoogleDnsIp);
    serv.sin_port = htons(kDnsPort);

    int err = connect(sock, (const sockaddr*) &serv, sizeof(serv));
    assert(err != -1);

    sockaddr_in name;
    socklen_t namelen = sizeof(name);
    err = getsockname(sock, (sockaddr*) &name, &namelen);
    assert(err != -1);

    const char* p = inet_ntop(AF_INET, &name.sin_addr, buffer, buflen);
    assert(p);
    close(sock);
}

