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

std::atomic<bool> ready{false};
std::vector<std::string> requests_tmp;
std::vector<std::string> requests;

Mixer::Mixer(std::string mixerip, std::vector<std::string> mixers, std::vector<std::string> mailboxes)
{
    dht::DhtRunner node;
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
    plugin = string(reinterpret_cast<char*>(this->public_key)) + ":" + mixer_ip;

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
                    
                    size_t pos = 0;
                    std::string token;
                    pos = mydata.find(":");
                    string pub = mydata.substr(0,pos);
                    string ip = mydata.erase(0, pos + string(":").length());
                    GiveMeDataForPublic(pub, ip);
                }
                return true; // keep looking for values
            },
            [=](bool success) {
                std::cout << "Getting mixers ready: " << (success ? "success" : "failure") << std::endl;
                done_cb(success);
            }
    );
    wait();

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
    s.StartListening();
    std::this_thread::sleep_until(std::chrono::system_clock::now() +
    std::chrono::hours(std::numeric_limits<int>::max()));
}

void Mixer::StartRoundAsMixer(){

    //Start a server in the background
    auto f = std::async(std::launch::async, StartServerInBackground);

    //Start a message listener in the background
    auto fL = std::async(std::launch::async, ListenForMessages);

    while(true){
        if(!ready){
            std::copy(s.msgs.begin(), s.msgs.end(), std::back_inserter(requests_tmp));
            s.msgs.clear();
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
            }
        }
        sleep(10);
        requests_tmp.clear();
        ready = true;
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

void ListenForMessages(){
    int sockfd; //to create socket
    int newsockfd; //to accept connection
    
    struct sockaddr_in serverAddress; //server receive on this address
    struct sockaddr_in clientAddress; //server sends to client on this address

    int n;
    char msg[MAXSZ];
    int clientAddressLength;
    int pid;

    //create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    //initialize the socket addresses
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(PORT);

    //bind the socket with the server address and port
    bind(sockfd, (struct sockaddr * ) & serverAddress, sizeof(serverAddress));

    //listen for connection from client
    // allow 5 pending connetions
    listen(sockfd, 5);

    while (1) {
        //parent process waiting to accept a new connection
        printf("\n*****Waitng to accept a connection:*****\n");
        clientAddressLength = sizeof(clientAddress);
        newsockfd = accept(sockfd, (struct sockaddr * ) &clientAddress, (socklen_t*) &clientAddressLength);
        printf("connected to client: %s\n", inet_ntoa(clientAddress.sin_addr));
        
        //child process is created for serving each new client
        pid = fork();
        if (pid == 0) //child process rec and send
        {
            //rceive from client
            while (1) {
                n = recv(newsockfd, msg, MAXSZ, 0);
                if (n == 0) {
                    close(newsockfd);
                    break;
                }
                if(msg == "publickeys"){
                    // Send the user all the public keys of the mixnodes
                    std::string str = ConvertMapToString(ipspub);
                    send(newsockfd, str.c_str(), sizeof(str), 0);
                }else{
                    requests.push_back(msg);
                    char* ack = "MessageRecieved";
                    send(newsockfd, ack, sizeof(ack), 0);
                     if(ready){
                        std::copy(requests.begin(), requests.end(), std::back_inserter(requests_tmp));
                        requests.clear();
                        ready = false;
                    }
                }
            } 
            exit(0);
         } else {
            close(newsockfd); //sock is closed BY PARENT
        }

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

