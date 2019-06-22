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
#include <vector>
#include <string>

using namespace std;

int main(){

    
    //Testing 
    // 127.0.0.1, 172.17.0.2, 172.17.0.3, 172.17.0.4
    
    string mixerip;
    vector<string> mixers; 
    vector<string> mailboxes;

    int amt;
    cout << "How many mixers do you have?" << endl;
    cin >> amt;
    while(amt--){
        string ip;
        cout << "Enter ip address" << endl;
        cin >> ip;
        mixers.push_back(ip);
    }

    // Runs the mixer m
    mixerip = mixers[0];
    Mixer m(mixerip,mixers,mailboxes);
    return 0;
}