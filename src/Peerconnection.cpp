#include "Peerconnection.hpp"

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
PeerConnection::PeerConnection()
{

    unsigned char ibuf[] = "congenial";
    unsigned char obuf[20];

    SHA1(ibuf, 9, obuf);
    auto tmpbuf = reinterpret_cast<char*>(obuf);
    memcpy(this->id, obuf, sizeof(obuf));

    GetPrimaryIp(this->myip, 20);
    cout << this->myip << endl;
    
    // listen on port 4222.
    this->node.run(4222, dht::crypto::generateIdentity(), true);

    //this->node.bootstrap("bootstrap.ring.cx", "4222");

    // put some data on the dht
    this->node.putSigned(string(this->id), string(this->myip));

    // get data from the dht
    this->node.get("other_unique_key", [](const std::vector<std::shared_ptr<dht::Value>>& values) {
        // Callback called when values are found
        for (const auto& value : values)
            std::cout << "Found value: " << *value << std::endl;
        return true; // return false to stop the search
    });

    this->node.join();
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

PeerConnection::~PeerConnection(){
    //this->node.shutdown();
}