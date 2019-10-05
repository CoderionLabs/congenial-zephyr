#include <sodiumwrap/sodiumtester.h>
#include <sodiumwrap/box_seal.h>
#include <sodiumwrap/keypair.h>
#include <sodiumwrap/allocator.h>

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

using namespace sodium;

bytes test(std::string& x, int N){


    bytes plainblob{ x.cbegin(), x.cend() };

    box_seal<> sb{};

    std::vector<sodium::keypair<>> mixers;
    for(int i = 0; i < N; i++){
        sodium::keypair<> mix{};
            std::cout << "WORKS" << std::endl;

        mixers.push_back(mix);
    }

    std::cout << "WORKS" << std::endl;
    bytes tmpenc = std::move(plainblob);
    for(auto x : mixers){
        tmpenc =  sb.encrypt(tmpenc, x.public_key());
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