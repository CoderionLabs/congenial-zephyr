/*
 * Copyright (c) 2019 Doku Enterprise
 * Author: Friedrich Doku
 * -----
 * Last Modified: Saturday April 13th 2019 8:41:54 am
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


#include "Mixer.hpp"

using namespace jsonrpc;


//GLOBALS...
HttpServer httpserver(8000);
MixerServer s(httpserver,
            JSONRPC_SERVER_V1V2);
map<string,string> ipspub;

std::atomic_bool dowork;
std::vector<std::string> requests;
std::vector<std::string> requests_tmp;
std::string MIXERIP;

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>


class session
    : public std::enable_shared_from_this < session > {
        public: session(tcp::socket socket): socket_(std::move(socket)) {}

        void start() {
            do_read();
        }

        private: void do_read() {
            auto self(shared_from_this());
            socket_.async_read_some(boost::asio::buffer(data_, max_length),
                [this, self](boost::system::error_code ec, std::size_t length) {
                    if (!ec) {
                        do_write(length);
                    }
                });
        }

        void do_write(std::size_t length) {
            std::string request(data_);
            auto self(shared_from_this());

            if (request.find("publickeys") != std::string::npos) {
                // Send the user all the public keys of the mixnodes
                std::string str = ConvertMapToString(ipspub);
                std::cout << "LENGTH OF PUBLIC KEYS: " << str.length() << std::endl;
                boost::asio::async_write(socket_, boost::asio::buffer(str, str.length()),
                    [this, self](boost::system::error_code ec, std::size_t /*length*/ ) {
                        if (!ec) {
                            do_read();
                        }
                    });
            } else {
                requests.push_back(request);
                char * ack = "MessageRecieved";
                boost::asio::async_write(socket_, boost::asio::buffer(ack, strlen(ack)),
                    [this, self](boost::system::error_code ec, std::size_t /*length*/ ) {
                        if (!ec) {
                            do_read();
                        }
                    });
                if (dowork.load()) {
                    std::cout << "COPYING MESSAGES" << std::endl;
                    std::copy(requests.begin(), requests.end(), std::back_inserter(requests_tmp));
                    requests.clear();
                    dowork = false;
                }
            }
        }

        tcp::socket socket_;
        enum {
            max_length = 1024
        };
        char data_[max_length];
    };

class server {
    public:
        server(boost::asio::io_context & io_context, short port): acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
            do_accept();
        }

    private:
        void do_accept() {
            acceptor_.async_accept(
                [this](boost::system::error_code ec, tcp::socket socket) {
                    if (!ec) {
                        std::make_shared<session>(std::move(socket))->start();
                    }

                    do_accept();
                });
        }

    tcp::acceptor acceptor_;
};

Mixer::Mixer(std::string mixerip, std::vector<std::string> mixers, std::vector<std::string> mailboxes)
{
    dht::DhtRunner node;
    MIXERIP = mixerip;
    auto mtx = std::make_shared<std::mutex>();
    auto cv = std::make_shared<std::condition_variable>();
    auto ready = std::make_shared<bool>(false);

    std::cout << "Creating Keys..." << std::endl;
    crypto_box_keypair(this->public_key, this->private_key);
    auto tmpid = this->node.getNodeId().to_c_str();
    std::cout << "DONE" << std::endl;
    
    memcpy(this->id, tmpid, sizeof(tmpid));    

    auto x = this->node.getPublicAddressStr();
    for(std::string w : x){
        this->myip.push_back(w);
    }
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
    plugin = string(reinterpret_cast<char*>(this->public_key)) + "_____________________________________________" + mixerip;

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
            std::cout << "Not everyone is ready yet sleeping" << std::endl;
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

                    GiveMeDataForPublic(pub, ip);
                }
                return true; // keep looking for values
            },
            [=](bool success) {
                std::cout << "Getting mixers ready: " << (success ? "success" : "failure") << std::endl;
                done_cb(success);
            }
    );

    auto mapstring = ConvertMapToString(ipspub);
    std::cout << "MAPSTRING START" << std::endl;
    std::cout << mapstring << std::endl;
    std::cout << "MAPSTRING END" << std::endl;

    auto mymap = ConvertStringToMap(mapstring);
    if(mymap == ipspub){
        std::cout << "WORKS MAPS ARE EQUAL" <<  std::endl;
    }
    wait();

    std::cout << "THESE ARE THE KEYS I HAVE START" <<  std::endl;
    for(auto x : ipspub){
         std::cout << x.first << " and " << x.second <<  std::endl;
    }
    std::cout << "THESE ARE THE KEYS I HAVE END" <<  std::endl;

    this->node.join();

    
    this->StartRoundAsMixer();
}

void GiveMeDataForPublic(std::string pub, std::string ip){
    ipspub[ip] = pub;
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
    //this->node.shutdown();
}

void StartServerInBackground(){
    if(s.StartListening()){
        std::cout << "BACKGROUND SERVER STARTED" << std::endl;
            std::this_thread::sleep_until(std::chrono::system_clock::now() +
            std::chrono::hours(std::numeric_limits<int>::max()));
    }
    if(s.StopListening()){
        std::cout << "BACKGROUND SERVER STOPPED" << std::endl;
    }
}

void Mixer::StartRoundAsMixer(){

    dowork = false;
    //Start a server in the background
    thread t(StartServerInBackground);
    //Start a message listener in the background
    thread t1 (ListenForMessages);
    t.detach();
    t1.detach();

    while(true){
        if(!dowork.load()){
            std::cout << "THIS WORKS" << std::endl;
            std::copy(s.msgs.begin(), s.msgs.end(), std::back_inserter(requests_tmp));
            s.msgs.clear();
            std::cout << "IM IN HERE " << requests_tmp.size() << std::endl;
            if(requests_tmp.size() != 0){
                std::mt19937 rng;
                rng.seed(std::random_device()());
                std::uniform_int_distribution<std::mt19937::result_type> dist6(1,10);

                auto x = dist6(rng);
                std::cout << "THIS IS IT" << std::endl;
                std::cout << requests_tmp[0] << std::endl;

                Shuffle<std::string> shu(requests_tmp, (int) x);

                // Strip off a layer of encryption and send to the next
                // mixer.
                for(auto x : shu.vec){
                    unsigned char* decrypted;
                    crypto_box_seal_open(decrypted, reinterpret_cast<const unsigned char*>(x.c_str()),
                    x.length(), this->public_key, this->private_key);

                    std::string conv = reinterpret_cast<char*>(decrypted);
                    std::cout << "THIS IS THE MESSAGE I GOT " << conv << std::endl;
                    auto pos = conv.find(":");
                    std::string nextmixer = conv.substr(0, pos);
                    conv.erase(0, pos + 1);

                    senddata(nextmixer, conv);
                }
                requests_tmp.clear();
            } 
            dowork = true;
        }
    }
}

// Send data to the next node
void senddata(std::string ip, std::string msg){
    HttpClient httpclient("http://" + ip + ":8000");
    MixerClient c(httpclient, JSONRPC_CLIENT_V2);

    try {
        c.getMessage(msg);
    } catch (JsonRpcException &e) {
        cerr << e.what() << endl;
    }
}

int ListenForMessages(){
    try {
        boost::asio::io_context io_context;
        server s(io_context, PORT);
        io_context.run();
    } catch (std::exception & e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
}

std::string ConvertMapToString(std::map<string,string> mymap){
    std::string result;
    for(auto const& x : mymap){
        result += "{";
        result += x.first;
        result += ":";
        result += x.second;
        result += "}";
        result += ",";
    }
    result.pop_back();
    return result;
}

std::map<string,string> ConvertStringToMap(std::string mapstring){
    std::map<string, string> result;
    while(!mapstring.empty()){
        auto pos = mapstring.find("{");
        auto pos1 = mapstring.find(":");
        string x = mapstring.substr(pos +1, (pos1 - pos) -1);

        auto pos2 = mapstring.find("}");
        string y = mapstring.substr(pos1 +1, (pos2 - pos1) -1);
        result[x] = y;
        try{
            mapstring.erase(pos,pos2 + 2);
        }catch(std::exception e){
            break;
        }
    }
    return result;
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

