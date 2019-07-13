#include <iostream>
#include <opendht.h>
#include <thread>
#include <string>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <zephyr/utils.hpp>

std::string ipspubstr;
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
std::atomic_bool imFirst{false};
std::atomic_bool havedata{false};

// If the infonode has been selected it will be the first
// node to upload the data into the DHT. Otherwise it will
// get it from the DHT because another node has already stored
// the data. A random mixer will be chosen every round to select
// a random infonode.

class session
    : public std::enable_shared_from_this <session> {
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

            if (request == "" && havedata.load()) {
                // Send the user all the public keys of the mixnodes
                std::string str = ipspubstr;
                length = str.length();
                std::cout << "LENGTH OF PUBLIC KEYS: " << str.length() << std::endl;
                boost::asio::async_write(socket_, boost::asio::buffer(str, length),
                    [this, self](std::error_code ec, std::size_t /*length*/) {
                        if (!ec) {
                            do_read();
                        }
                });
            } else {
               ipspubstr = request;
               imFirst = true;
            }
        }

        tcp::socket socket_;
        enum {
            max_length = 1024
        };
        char data_[max_length];
    };

class server
{
public:
  server(boost::asio::io_context& io_context, short port)
    : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)),
      socket_(io_context)
  {
    do_accept();
  }

private:
  void do_accept()
  {
    acceptor_.async_accept(socket_,
        [this](std::error_code ec)
        {
          if (!ec)
          {
            std::make_shared<session>(std::move(socket_))->start();
          }

          do_accept();
        });
  }

  tcp::acceptor acceptor_;
  tcp::socket socket_;
};

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

    if(imFirst.load()){
        // Put the map data into the DHT
        // since we are first this time.
        node.put("mapdata", dht::Value((const uint8_t *)ipspubstr.data(), ipspubstr.size()), [=](bool success) {
            std::cout << "Put: ";
            done_cb(success);
        });

        // blocking to see the result of put
        wait();
        havedata = true;
    }

    // Otherwise we keep looking for the mapdata
    while(!havedata){
        // get data from the dht
        node.get("mapdata",
             [](const std::vector<std::shared_ptr<dht::Value>> &values) {
                 // Callback called whsen values are found
                 for (const auto &value : values){
                     std::string mapdata{value->data.begin(), value->data.end()};
                     std::cout << "Found value: " << mapdata << std::endl;
                     
                 }
                 return true; // return false to stop the search
             },
             [=](bool success) {
                 std::cout << "Get: ";
                 done_cb(success);
                 havedata = true;
             });
        wait();
    }
    
    // wait for dht threads to end
    // node.join();
}

int main(int argc, char* argv[]){
    std::thread t1(dhtstart);
    t1.detach();
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: infonode <port>\n";
            return 1;
        }
        
        boost::asio::io_context io_context;
        server s(io_context, std::atoi(argv[1]));
        io_context.run();
    }catch (std::exception& e){
        std::cerr << "Exception: " << e.what() << "\n";
    }
}