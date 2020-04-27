
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
#include <execution>
#include <ctime>
#include <chrono>

using namespace sodium;

std::chrono::time_point<std::chrono::system_clock> start;
bytes test(std::string& x, int N){
    bytes plainblob{ x.cbegin(), x.cend() };
    box_seal<> sb{};

    sodium::keypair<> mix{};    
    

    std::vector<bytes> foo;

    for(int i = 0; i < N; i++){
        bytes tmpenc =  sb.encrypt(plainblob, mix.public_key());


        std::string tt{tmpenc.cbegin(), tmpenc.cend()};
        //tt +="poop";

        bytes tempenctmp{tt.cbegin(), tt.cend()};
        foo.push_back(tempenctmp);
    }

    start = std::chrono::system_clock::now();
    bytes rez;
    std::for_each(
    std::execution::par_unseq,
    foo.begin(),
    foo.end(),
    [&](auto&& item)
    {
        rez = sb.decrypt(item, mix.private_key(), mix.public_key());
    });

    // for(auto item : foo){
    //     rez = sb.decrypt(item, mix.private_key(), mix.public_key());
    // }

    return rez;
}

int main()
{
    sodium_init();

    std::string message;
    std::cout << "ENTER MESSAGE" << std::endl;
    std::cin >> message;
    
    bytes tmp = test(message,1000);
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;
    std::time_t end_time = std::chrono::system_clock::to_time_t(end);

    std::cout << "finished computation at " << std::ctime(&end_time)
              << "elapsed time: " << elapsed_seconds.count() << "s\n";
    std::string x{tmp.begin(), tmp.end()};
    std::cout << x << std::endl;
}


