/*
 * Copyright (c) 2019 Doku Enterprise
 * Author: Friedrich Doku
 * -----
 * Last Modified: Friday April 5th 2019 10:24:17 am
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


#include <zephyr/Mixer.hpp>
#include <zephyr/utils.hpp>
#include <vector>
#include <string>

using namespace std;

int main(int argc, char* argv[]){
    if (argc != 2){
        std::cerr << "Usage: mixserver MYIPV4ADDRESS CONFIG_FLIE_PATH\n";
        return 1;
    }
    auto config = get_config_info(argv[2]);
    std::string mixerip = argv[1];

    //Testing 
    // 127.0.0.1, 172.17.0.2, 172.17.0.3, 172.17.0.4
    // Runs the mixer m
    Mixer m(mixerip,config[0],config[1],argv[2]);
    return 0;
}