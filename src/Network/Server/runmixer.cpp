#include <zephyr/Mixer.hpp>
#include <vector>
#include <string>

int main(){

    std::string mixerip;
    std::vector<std::string> mixers;
    std::vector<std::string> mailboxes;

    // Runs the mixer m
    Mixer m(mixerip,mixers,mailboxes);
    return 0;
}