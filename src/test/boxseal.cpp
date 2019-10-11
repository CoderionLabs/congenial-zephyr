#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <zephyr/utils.hpp>
#include <string>
#include <vector>

using namespace sodium;

bytes test(std::string& x, int N){
    bytes plainblob{ x.cbegin(), x.cend() };
    std::vector<std::string> keys;

    box_seal<> sb{};

    std::vector<sodium::keypair<>> mixers;
    for(int i = 0; i < N; i++){
        sodium::keypair<> mix{};
        std::cout << "WORKS" << std::endl;

        auto tmpstrkey = serial_box_key(mix.public_key());
        keys.push_back(tmpstrkey);

        mixers.push_back(mix);
    }

    std::cout << "WORKS" << std::endl;
    bytes tmpenc = std::move(plainblob);
    int i = 0;
    for(auto &x : mixers){
        auto t = keys[i];
        x.public_key_ = deserial_box_key(t);
        tmpenc =  sb.encrypt(tmpenc, x.public_key());
        i++;
    }

    for(int i = (N-1); i > -1; i--){
        tmpenc = sb.decrypt(tmpenc, mixers[i].private_key(), mixers[i].public_key());
    }
    return tmpenc;
}

int main()
{
    sodium_init();

    std::string message;
    std::cout << "ENTER MESSAGE" << std::endl;
    std::cin >> message;
    bytes tmp = test(message,10);
    std::string x{tmp.begin(), tmp.end()};
    std::cout << x << std::endl;
}