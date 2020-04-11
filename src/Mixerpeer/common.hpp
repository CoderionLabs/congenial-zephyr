/*
 * Copyright (c) 2020 Mutex Unlocked
 * Author: Friedrich Doku
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

#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <iostream>
#include <string.h>
#include <cassert>
#include <string>
#include <mutex> 
#include <array>
#include <map>
#include <netinet/in.h>
#include <fstream>

auto write_string_to_file(std::string endpoints){
    std::ofstream out;
    out.open("data.txt", std::ios_base::app);
    if(out.fail()) return false;
    out << endpoints << std::endl;
    out.close();
    return true;
}

auto parse_get(std::string get){
    return get.erase(0,3);
}

auto file_to_map(){
    std::map<std::string, std::pair<std::string,std::string>> endpoints;

    std::ifstream infile("data.txt");
    std::string line;
    while(std::getline(infile,line)){
        std::string cut("CUTHERE");
        size_t found = line.find(cut);

        auto publicip = line.substr(found + cut.size());
        line.erase(line.begin() + found, line.end());

        endpoints[line] = std::make_pair(line, std::move(publicip));
    }
    return endpoints;
}