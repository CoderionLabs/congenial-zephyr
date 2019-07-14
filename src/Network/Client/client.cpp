/*
 * Copyright (c) 2019 Doku Enterprise
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


#include <zephyr/utils.hpp>
#include <zephyr/pkgc.hpp>
#include <zephyr/pkg.hpp>
#include <zephyr/Mixer.hpp>
#include <zephyr/utils.hpp>
#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sodium.h>
extern "C"{
    #include <sibe/ibe.h>
    #include <sibe/ibe_progs.h>
}

CONF_CTX *cnfctx;
params_t params;

using namespace std;
// Mixers, Mailboxes, PKGS

void error(const char *msg)
{
    perror(msg);
    exit(0);
}
std::string talktonode(std::string ip, std::string msg);

int main(){
    string msg,email,key, params, filepath;
    cout << "Enter message" << endl;
    cin >> msg;
    cout << "Enter recievers email" << endl;
    cin >> email;
    cout << "Enter config file path" << endl;
    cin >> filepath;

    // Load the configuration file
    //string filepath = "";
    vector<vector<std::string>> vec;
    vec = get_config_info(filepath);
    cout << "PASSED HERE" << endl;
    cout << "Asking " << vec[2][1] << " for data" << endl;
    auto x = getkeysfrompkg(vec[2][1], to_string(8080), email);
    cout << "GOT IT" << endl;
    // Get your private key
    byte_string_t keyb;
    params_t paramsb;
    IBE_init();
    cout << x[0] << endl;
    cout << x[1] << endl;
    std::string key_serial_tmp = std::move(x[0]);
    std::string param_serial_tmp = std::move(x[1]);
    deserialize_bytestring(key_serial_tmp, keyb);
    deserialize_params(param_serial_tmp, paramsb);

    FILE * filePointer; 
    filePointer = fopen("params.txt","w+");
    params_out(filePointer, paramsb);
    fclose(filePointer);

    cout <<  "SIZE OF FULL DATA IS " << sizeof(keyb) + sizeof(paramsb) << endl;
    // Encrypt message for user
    cout << "MADE IT HERE 2" << endl;
    std::string encdata = pkg_encrypt("fried", paramsb, "LIfe is what we make of it.");
    cout << "MADE IT HERE FINISHED" << endl;
    // Select random mixer and send data to it
    int num = rand() % vec[3].size() -1;
    string ip = vec[3][num];
    cout << "Talking to " << ip << endl;
    auto data = talktonode(ip, "8080","");
    cout << "DATA START" << endl;
    //cout << data << endl << "DATA END" << endl;
    auto map = ConvertStringToMap(data);
    cout << "I'M NOT HAVING TROUBLE!" << endl;
    auto mixenc = map[ip];
    
    cout << "START OF MIXENC" << endl;
    cout << mixenc << endl;
    cout << "END OF MIXENC" << endl;

    cout << "Talking to " << ip << endl;
    // Seal with crypto sercret box also append destination address to it data:ip
     // Send it back to the mixer

    // int CIPHERTEXT_LEN = crypto_box_SEALBYTES + msg.length();
    // unsigned char ciphertext[CIPHERTEXT_LEN];
    // crypto_box_seal(ciphertext, reinterpret_cast<unsigned char*>(&msg[0]), msg.length(),
    //   reinterpret_cast<unsigned char*>(&mixenc[0]));
    // auto gotbuf = talktonode(ip, reinterpret_cast<char*>(ciphertext));
    // cout << gotbuf << endl;
    return 0;
}