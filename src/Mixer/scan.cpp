/*
 * Copyright (c) 2019 Doku Enterprise
 * Author: Friedrich Doku
 * -----
 * Last Modified: Saturday May 4th 2019 1:44:23 pm
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

#include "scan.hpp"

std::vector<std::vector<std::string>> get_config_info(std::string filename){
    std::ifstream in(filename);
    if(in.fail()){
        std::cout << "Failed to open file" << std::endl;
    }
    std::string data; std::map<std::string,int> types;
    std::vector<std::vector<std::string>> myvec;
    types["MIXERS"] = 1;
    types["MAILBOXES"] = 2;
    types["PKGS"] = 3;
    int arr = 0;
    while(in >> data){
        int tmp = types[data] -1;
        if(tmp != -1){
            arr = tmp;
        }
        myvec[arr].push_back(data);
    }
}
//EXAMPLE
//MIXERS
//1.0.012
//10.0.0.12
//127.0.0.1
//MAILBOXES
//IP
//IP
//IP
//PKGS
//IP
//IP
//IP