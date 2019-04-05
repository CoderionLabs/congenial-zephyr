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
    cout << "How many mixer do you have?" << endl;
    cin >> amt;
    while(amt--){
        string ip;
        cout << "Enter ip address" << endl;
        cin >> ip;
        mixers.push_back(ip);
    }

    // Runs the mixer m
    Mixer m(mixerip,mixers,mailboxes);
    return 0;
}