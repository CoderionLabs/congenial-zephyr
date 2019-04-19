#include "scan.hpp"

std::vector<std::vector<std::string>> get_config_info(std::string filename){
    std::ifstream in(filename);
    if(in.fail()){
        std::cout << "Failed to open file" << std::endl;
    }
    std::string data; std::map<std::string,int> types;
    std::vector<std::vector<std::string>> myvec;
    types["MIXERS"] = 0;
    types["MAILBOXES"] = 1;
    types["PKGS"] = 2;
    int arr;
    while(in >> data){
        int tmp = types[data];
        if(tmp != 0){
            arr = tmp;
        }
        myvec[arr].push_back(data);
    }
}