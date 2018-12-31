#include "Mixer.hpp"

using namespace std;
/* TODO:  this is the precedure
    1. I need to computer a unique id for each node 
        using a hash function with almost no colloions 
    2. I have to computer public-private key pairs for each node
    3. I need to upload the public keys into the DHT but
        use a public key certificate
    4. create an algorithm that pulls messages from a server
    5. The server will randomize the ids and compute a route
    6. The mixnet will then take in messages and encrypt each one
        adding layers of encryption and noise to the message
    7. Once this is finished  */
// Setup the dht

//Globals

std::vector<std::string> msgs;

//TODO: Implement Bully leader election
Mixer::Mixer()
{

    /*unsigned char ibuf[] = "congenial";
    unsigned char obuf[20];

    SHA1(ibuf, 9, obuf);
    auto tmpbuf = reinterpret_cast<char*>(obuf);
    memcpy(this->id, obuf, sizeof(obuf));*/


    crypto_box_keypair(recipient_pk, recipient_sk);


    char hostbuffer[256]; 
    char *IPbuffer; 
    struct hostent *host_entry; 
  
    // To retrieve host information 
    host_entry = gethostbyname(hostbuffer); 
    checkHostEntry(host_entry); 
  
    // To convert an Internet network 
    // address into ASCII string 
    IPbuffer = inet_ntoa(*((struct in_addr*) 
                           host_entry->h_addr_list[0])); 
  
    // Get the public_ip of the current server
    this->public_ip = IPbuffer;

    bool is_the_first = false;
    auto tmpid = this->node.getNodeId().to_c_str();
    
    memcpy(this->id, tmpid, sizeof(tmpid));    

    auto x = this->node.getPublicAddressStr();
    for(std::string w : x){
        this->myip.push_back(w);
    }
    
    // listen on port 4222.
    this->node.run(4222, dht::crypto::generateIdentity(), true);

    // The first node in the network will not use a bootstrap
    // node to join the network. I will use hardcoded addresses
    // so that a node can join the network.
    this->node.bootstrap("142.93.188.57", "4222");

    // put some data on the dht
    this->node.putSigned("IP_LIST", this->myip, [](bool ok){
        if(not ok){
            cout << "Failed to add adress" << endl;
        }
    });

    this->node.get("FIRST", [&](const std::vector<std::shared_ptr<dht::Value>>& values) {
        // Callback called when values are found
        for (const auto& value : values){
            std::cout << "Found value: " << *value << std::endl;
            is_the_first = true;
            return true;
        }
        return false; // return false to stop the search
    });

    // Get some known nodes from other nodes
    this->node.get("IP_LIST", [](const std::vector<std::shared_ptr<dht::Value>>& values) {
        // Callback called when values are found
        for (const auto& value : values)
            std::cout << "Found value: " << *value << std::endl;
        return true; // return false to stop the search
    });

    this->node.join();
    std::thread t1(ListenForMessages);
    t1.join();

    if(is_the_first){
        this->StartRoundAsCoordinator();
    }else{
        this->StartRoundAsMixer();
    }
}

Mixer::~Mixer(){
    // Before leaving add the list of known nodes to a
    // file and also put them in the DHT
    ofstream out("nodes.dems");
    for(auto x : this->knownNodeAdresses){
        out << x << endl;
    }
    out.close();

    this->node.putSigned("ROUTING_TABLE:" + string(this->id), this->knownNodeAdresses, [](bool ok){
        if(not ok){
            cout << "Failed to publish known nodes" << endl;
        }
    });
    //this->node.shutdown();
}

void GetMessageFromClients(std::string message){
    msgs.push_back(message);
}

void Mixer::StartRoundAsMixer(){
    rpc::client client(this->public_ip, rpc::constants::DEFAULT_PORT);
    for(auto x : msgs){
        client.send("GetMessageFromClients", x);
    }
    //std::cout << "The result is: " << result << std::endl;
}

void Mixer::StartRoundAsCoordinator(){
     // Start another thread that listens for incoming messages

    //Sets the deadline to 20 seconds
    // TODO: in the future set this acording to the amount
    // of connected clients.
    //this->deadline = 20;
    
    // TODO: Download messages from each server and shuffle them
    // Send an rpc to all servers and tell them to send their messages
    // to me 
    
    // Right now assume the messages have already been encrypted with the
    // receipents public keys

    // Create a server that listens on port 8080
    rpc::server srv(this->public_ip, rpc::constants::DEFAULT_PORT);
    srv.bind("GetMessageFromClients", &GetMessageFromClients);
    srv.run();

    std::vector<unsigned char*> ciphers;

    // Encrypt each message with public key
    for(string x : msgs){
        unsigned char ciphertext[CIPHERTEXT_LEN];
        crypto_box_seal(ciphertext, x.c_str(), x.length(), this->private_key);
        cipher.push_back(ciphertext);
    }

    // Shuffle the messages
    Shuffle shu;
    shu.Shuffle(ciphers, 5);

    // Send them to the next node the next node.
    // The next node will do the same thing 

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
                msgs.push_back(msg);

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

