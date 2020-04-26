
/*
 * Copyright (c) 2020 Mutex Unlocked
 * Author: Friedrich Doku
 * -----
 * Last Modified: Friday May 24th 2020 1:39:09 pm
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



#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <zephyr/utils.hpp>
#include <string>
#include <vector>
#include <ctime>
#include <chrono>

using namespace sodium;

bytes test(std::string& x, int N){
    bytes plainblob{ x.cbegin(), x.cend() };
    std::vector<std::string> keys;

    box_seal<> sb{};

    std::vector<sodium::keypair<>> mixers;
    for(int i = 0; i < N; i++){
        sodium::keypair<> mix{};
        std::cout << "WORKS" << std::endl;
        
        auto tmpstrkey = setupkey(mix.public_key(),"127.0.0.1");
        keys.push_back(tmpstrkey);
        mixers.push_back(mix);
    }

    std::cout << "WORKS" << std::endl;
    bytes tmpenc = std::move(plainblob);
    int i = 0;
    for(auto &x : mixers){
        auto t = keys[i];
        auto pair = getkeyfromtxt(t);
        x.public_key_ = pair.second;
        tmpenc =  sb.encrypt(tmpenc, x.public_key());


        std::string tt{tmpenc.cbegin(), tmpenc.cend()};
        //tt +="poop";

        bytes tempenctmp{tt.cbegin(), tt.cend()};
        tmpenc = tempenctmp;

        i++;
    }

    for(int i = (N-1); i > -1; i--){
        // tmpenc.pop_back();
        // tmpenc.pop_back();
        // tmpenc.pop_back();
        // tmpenc.pop_back();
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
    auto start = std::chrono::system_clock::now();
    bytes tmp = test(message,1000);
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;
    std::time_t end_time = std::chrono::system_clock::to_time_t(end);

    std::cout << "finished computation at " << std::ctime(&end_time)
              << "elapsed time: " << elapsed_seconds.count() << "s\n";
    std::string x{tmp.begin(), tmp.end()};
    std::cout << x << std::endl;
}