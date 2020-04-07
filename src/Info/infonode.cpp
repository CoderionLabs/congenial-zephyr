#include <iostream>
#include <opendht.h>
#include <thread>
#include <string>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <zephyr/utils.hpp>

#include <zephyr/nodeserver.hpp>
#include <zephyr/nodeclient.hpp>


std::vector<std::string> msgtmp;
std::vector<std::string> outbox;
std::vector<std::string> msgtmp2;

std::vector<std::string> needrequests;
std::vector<std::string> keys;
std::string INFONODEIP;
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
bool havedata = false;
bool pushed = false;

// If the infonode has been selected it will be the first
// node to upload the data into the DHT. Otherwise it will
// get it from the DHT because another node has already stored
// the data. A random mixer will be chosen every round to select
// a random infonode.

void RunServerInBackground(){
    std::string server_address(INFONODEIP + ":50051");
    NodeImpl service;

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;
    server->Wait();
}

void sendkeys(std::vector<std::string> needreq){
    for(auto ipaddr : needreq){
        NodeClient inforeq(
        grpc::CreateChannel(ipaddr + ":50051",
                          grpc::InsecureChannelCredentials()));

        std::cout << "-------------- GetMessages --------------" << std::endl;
        for(auto k : keys){
            node::Msg tosend;
            tosend.set_data(k);
            inforeq.data.push_back(tosend);
        }
       
        inforeq.PutMessages();
    }
}

void dhtstart()
{

    dht::DhtRunner node;
    auto mtx = std::make_shared<std::mutex>();
    auto cv = std::make_shared<std::condition_variable>();
    auto ready = std::make_shared<bool>(false);

    // Launch a dht node on a new thread, using a
    // generated RSA key pair, and listen on port 4222.
    node.run(4222, dht::crypto::generateIdentity(), true);

    // Join the network through any running node,
    // here using a known bootstrap node.
    node.bootstrap("bootstrap.ring.cx", "4222");

    auto wait = [=] {
        *ready = true;
        std::unique_lock<std::mutex> lk(*mtx);
        cv->wait(lk);
        *ready = false;
    };
    auto done_cb = [=](bool success) {
        if (success){
            std::cout << "success!" << std::endl;
        }else{
            std::cout << "failed..." << std::endl;
        }
        std::unique_lock<std::mutex> lk(*mtx);
        cv->wait(lk, [=] { return *ready; });
        cv->notify_one();
    };

    // Otherwise we keep looking for the mapdata
    while(true){
        // get data from the dht
        if(!msgtmp.empty()){
            msgtmp2 = msgtmp;
            msgtmp.clear();
            for(auto x : msgtmp2){
                // if(x.find("NEED") != std::string::npos){
                //     // REMOVE NEED AND SEND TO IP
                //     std::cout << "I GOT A REQUEST" << std::endl;
                //     std::string tmp = x;
                //     auto pos = tmp.find("NEED");
                //     tmp = tmp.substr(pos + 4);
                //     needrequests.push_back(tmp);
                // }else{
                //     std::cout << "I GOT A KEY" << std::endl;
                //     keys.push_back(x);
                //     havedata = true;
                // }
                outbox.push_back(x);
                keys.push_back(x);
                havedata = true;
            }
            msgtmp2.clear();
        }

        // if(!needrequests.empty()){
        //     sendkeys(needrequests);
        //     needrequests.clear();
        // }

        if(havedata && (pushed == false)){
            // Put the map data into the DHT
            // since we are first this time.
            for(auto k : keys){
                 node.put("keys", dht::Value((const uint8_t *)k.data(), k.size()), [=](bool success) {
                std::cout << "Put: ";
                done_cb(success);
            });
            // blocking to see the result of put
            wait();
            pushed = true;
            }
           
        }

        if(!havedata){
            node.get("keys",
             [](const std::vector<std::shared_ptr<dht::Value>> &values) {
                 // Callback called whsen values are found
                 for (const auto &value : values){
                     std::string datakey{value->data.begin(), value->data.end()};
                     std::cout << "Found keys: not printing" << std::endl;
                     if(datakey.size() > 0){
                         keys.push_back(datakey);
                         outbox.push_back(datakey);
                         havedata = true;
                         pushed = true;
                         std::cout << "ALL SET UP" << std::endl;
                     }
                     
                 }
                 return true; // return false to stop the search
             },
             [=](bool success) {
                std::cout << "Getting keys ready: " << (success ? "success" : "failure") << std::endl;
                 done_cb(success);
             });
            wait();
        }
    }
    
    // wait for dht threads to end
    // node.join();
}

int main(int argc, char* argv[]){
    std::thread t1(dhtstart);
    t1.detach();

    if (argc != 2){
        std::cerr << "Usage: infonode INFONODEIP\n";
        return 1;
    }
    INFONODEIP = argv[1];
    std::cout << "RUNNING ON " << INFONODEIP << std::endl;
    RunServerInBackground();
}