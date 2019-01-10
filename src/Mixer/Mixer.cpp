#include "Mixer.hpp"

using namespace std;
using namespace jsonrpc;

map<string,string> ipspub;

//TODO: Implement Bully leader election
Mixer::Mixer(std::string mixerip, std::vector<std::string> mixers, std::vector<std::string> mailboxes)
{

    crypto_box_keypair(this->public_key, this->private_key);
    auto tmpid = this->node.getNodeId().to_c_str();
    
    memcpy(this->id, tmpid, sizeof(tmpid));    

    auto x = this->node.getPublicAddressStr();
    for(std::string w : x){
        this->myip.push_back(w);
    }
    
    // listen on port 4222.
    this->node.run(4222, dht::crypto::generateIdentity(), true);

    string plugin; const char* r = "ready";
    plugin = string(reinterpret_cast<char*>(this->public_key)) + ":" + mixer_ip;

    this->node.put(
        dht::InfoHash::get("publickeys"),
        dht::Value((const uint8_t*)plugin.c_str(), sizeof(plugin))
    );

    this->node.put(
        dht::InfoHash::get("ready"),
        dht::Value((const uint8_t*) r)
    );

    // Wait for all the other nodes to be ready
    while(true){
        this->node.get(
            dht::InfoHash::get("ready"),
            [&](const std::vector<std::shared_ptr<dht::Value>>& values) {
                for (const auto& v : values)
                    this->readymixers++;
                return true; // keep looking for values
            },
            [](bool success) {
                std::cout << "Getting mixers ready: " << (success ? "success" : "failure") << std::endl;
            }
        );

        if(this->readymixers == mixers.size()){
            break;
        }else{
            this->readymixers = 0;
        }
    }

    //Get the public keys still work to do 
    this->node.get(
        dht::InfoHash::get("publickeys"),
            [](const std::vector<std::shared_ptr<dht::Value>>& values) {
                for (const auto& v : values)
                    std::cout << "Got value: " << *v << std::endl

                    string x = string(reinterpret_cast<char*>(v.get());

                    size_t pos = 0;
                    std::string token;
                    pos = x.find(":");
                    string pub = x.substr(0,pos);
                    string ip = x.erase(0, pos + string(":").length());
                    GiveMeDataForPublic(pub, ip);
                return true; // keep looking for values
            },
            [](bool success) {
                std::cout << "Getting mixers ready: " << (success ? "success" : "failure") << std::endl;
            }
    );

    // The first node in the network will not use a bootstrap
    // node to join the network. I will use hardcoded addresses
    // so that a node can join the network.
    this->node.bootstrap("142.93.188.57", "4222");

    this->node.putSigned("IP_LIST", this->mixer_ip, [](bool ok){
        if(not ok){
            cout << "Failed to add adress" << endl;
        }
    });

    this->node.get("FIRST", [&](const std::vector<std::shared_ptr<dht::Value>>& values) {
        // Callback called when values are found
        for (const auto& value : values){
            std::cout << "Found value: " << *value << std::endl;
            return true;
        }
        this->is_the_first = true;
        return false; // return false to stop the search
    });

    // If we are the first tell other nodes to back off
    if(this->is_the_first){
        this->node.putSigned("FIRST", this->mixer_ip, [](bool ok){
            if(not ok){
                cout << "Failed to add adress" << endl;
            }
        });
    }

    this->node.join();
    std::thread t1(ListenForMessages);
    t1.join();

    if(this->is_the_first){
        this->StartRoundAsCoordinator();
    }else{
        this->StartRoundAsMixer();
    }
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
    //this->node.shutdown();
}

// TODO: has to wait for other server
// figure out how to take turns
void Mixer::StartRoundAsMixer(){
    HttpServer httpserver(8000);
    MixerServer s(httpserver,
                 JSONRPC_SERVER_V1V2);
    s.StartListening();
    //std::cout << "The result is: " << result << std::endl;
}

void Mixer::StartRoundAsCoordinator(){
    // Connect to a server
    if(!this->is_the_first){
        this->StartRoundAsMixer();
    }

    HttpServer httpserver(8000);
    MixerServer s(httpserver,
                 JSONRPC_SERVER_V1V2);
    s.StartListening();

    //Ask for messages
    for(std::string ip : this->mixers){
        bool go = true;
        while(go){
            HttpClient httpclient("http://" + ip + ":8000");
            MixerClient c(httpclient, JSONRPC_CLIENT_V2);

            try {
               auto msg = c.request("messages");
               if(msg == "0") go = false;
               this->messages.push_back(msg);
            } catch (JsonRpcException &e) {
                cerr << e.what() << endl;
            }
        }
    }

    std::vector<unsigned char*> ciphers;

    // Encrypt each message with public key
    for(string x : this->messages){
        unsigned char ciphertext[crypto_box_SEALBYTES + x.size()];
        crypto_box_seal(ciphertext, reinterpret_cast<const unsigned char*>(x.c_str()),
         x.length(), this->private_key);
        ciphers.push_back(ciphertext);
    }

    // Shuffle the messages
    auto v1 = rand() % RAND_MAX;
    Shuffle shu(ciphers, v1);
    
    // Find a new mixnet server
    std::string newNodeIp;
    for(std::string ip : this->mixers){
        bool go = true;
        while(go){
            HttpClient httpclient("http://" + ip + ":8000");
            MixerClient c(httpclient, JSONRPC_CLIENT_V2);

            try {
               auto msg = c.request("nextnode");
               if(msg == "yes"){
                   go = false;
                   newNodeIp = ip;
               }
            } catch (JsonRpcException &e) {
                cerr << e.what() << endl;
            }
        }
    }
    if(!newNodeIp.empty()){
        senddata(newNodeIp, this->messages);
    }else{
        // Do nothing for now
    }
    s.StopListening();
}

// Send data to the next node
void senddata(std::string ip, std::vector<std::string> msgs){
    HttpClient httpclient("http://" + ip + ":8000");
    MixerClient c(httpclient, JSONRPC_CLIENT_V2);

    try {
        for(auto s : msgs){
            auto result = c.getMessage(s);
        }
    } catch (JsonRpcException &e) {
        cerr << e.what() << endl;
    }
}

void Mixer::ListenForMessages(){
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
    memset( & serverAddress, 0, sizeof(serverAddress));
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
        
        //child process is created for serving each new clients
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
                this->messages.push_back(msg);

                char* ack = "Got your message";
                send(newsockfd, ack, sizeof(ack), 0);
            } 
            exit(0);
         } else {
            close(newsockfd); //sock is closed BY PARENT
        }
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

