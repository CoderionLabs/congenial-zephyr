#include "Mixer.hpp"

using namespace jsonrpc;


//GLOBALS...
HttpServer httpserver(8000);
MixerServer s(httpserver,
            JSONRPC_SERVER_V1V2);
map<string,string> ipspub;
std::vector<std::string> requests;

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

    // The first node in the network will not use a bootstrap
    // node to join the network. Mixer address will be loaded from the 
    // config file. The first mixer in the list will be used as a bootstrap node
    this->node.bootstrap(mixers[0], "4222");

    string plugin; string r = "ready";
    plugin = string(reinterpret_cast<char*>(this->public_key)) + ":" + mixer_ip;

    this->node.put(
        dht::InfoHash::get("publickeys"),
        dht::Value((const uint8_t*)plugin.data(), sizeof(plugin))
    );

    this->node.put(
        dht::InfoHash::get("ready"),
        dht::Value((const uint8_t*) r.data(), r.size())
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
            [](bool success) {
                std::cout << "Getting mixers ready: " << (success ? "success" : "failure") << std::endl;
            }
    );

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


    // Decrypt all the requests and send them to their
    // approrite mixers
    while(true){
        std::copy(s.msgs.begin(), s.msgs.end(), std::back_inserter(requests));
        s.msgs.clear();
        if(requests.size() != 0){
            auto tmprequest = requests;
            requests.clear();

            std::mt19937 rng;
            rng.seed(std::random_device()());
            std::uniform_int_distribution<std::mt19937::result_type> dist6(1,10);

            auto x = dist6(rng);

            Shuffle<std::string> shu(tmprequest, (int) x);

            // Strip off a layer of encryption and send to the next
            // mixer.
            for(auto x : shu.vec){
                unsigned char* decrypted;
                crypto_box_seal_open(decrypted, reinterpret_cast<const unsigned char*>(x.c_str()),
                x.length(), this->public_key, this->private_key);

                std::string conv = reinterpret_cast<char*>(decrypted);
                auto pos = conv.find(":");
                std::string nextmixer = conv.substr(0, pos);
                conv.erase(0, pos + 1);

                senddata(nextmixer, conv);
            }
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

