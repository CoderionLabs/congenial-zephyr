#include <zephyr/scan.hpp>
#include <zephyr/pkgc.hpp>
#include <iostream>
#include <vector>

using namespace std;

int main(){
    string msg,email,key, params;
    cout << "Enter message" << endl;
    cin >> msg;
    cout << "Enter email" << endl;
    cin >> email;

    // Load the configuration file
    string filepath = "";
    vector<vector<std::string>> vec;
    get_config_info(vec, filepath);
    getkeysfrompkg(vec[2][0], 55555, email);
    cout << "Now copy the KEY here" << endl;
    cin >> key;
    cout << "Now copy the PARAMS here" << endl;
    cin >> params;
    // TODO: Ask the PKG for the recievers public key
    

    return 0;
}