
#include <zepher/Mixer.hpp>

using namespace std;

int main(){
    Mixer m;
    char *arr = (char*)malloc(42);
    GetPrimaryIp(arr, sizeof(arr));
    cout << arr << endl;
    return 0;
}