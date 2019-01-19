// Just a test to prove correctness
#include <iostream>
#include <cassert>
#include <zepher/pkg.hpp>

int main(){
    using namespace std;

    std::string message = "I love coding!";

    PKG p; char xor_result[SIZE];
    p.setup(5,5);
    element_t public_key, private_key;
    p.extract(public_key, private_key, "FriedrichDoku");
    
    auto str = encrypt(message,"Stanford",p.P, p.Ppub,p.pairing);
    auto result = decrypt(private_key,str,p.pairing);

    cout << result << endl;
    cout << str << endl;
}