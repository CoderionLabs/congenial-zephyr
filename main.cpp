
#include <zepher/Peerconnection.hpp>

using namespace std;

int main(){
    PeerConnection m;
    char *arr = (char*)malloc(42);
    GetPrimaryIp(arr, sizeof(arr));
    cout << arr << endl;
    return 0;
}