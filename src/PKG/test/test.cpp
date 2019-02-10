// Just a test to prove correctness
//#define MESSAGE ((const unsigned char *) "The")
//#define MESSAGE_LEN 3
//#define CIPHERTEXT_LEN (crypto_secretbox_MACBYTES + MESSAGE_LEN)

#include <iostream>
#include <cassert>
#include <pbc/pbc.h>
#include <pbc/pbc_field.h>
#include <zepher/pkg.hpp>

int main(){
    using namespace std;
    
    // Test Correctness
    element_t public_key, private_key;
    element_t ad_public_key, ad_private_key;  
    std::string message = "have so far to go";
    std::string id = "friedrichdoku";
    std::string ad = "powerhouse@gmail.com";

    PKG p;
    p.setup(5,5);
    p.extract(public_key, private_key, id);
    p.extract(ad_public_key, ad_private_key, ad);

    // TODO: Fix problem with the encryption it doesn't
    // matter for some reason which id I put in.
    // The result will be the same both can decrypt the 
    // message. 
    auto str = encrypt(message,id,p.P, p.Ppub,p.pairing);
    auto result = decrypt(private_key,str,p.pairing);
    auto result2 = decrypt(ad_private_key,str,p.pairing);

   // assert(ad_private_key->field == ad_private_key->field);

    cout << message << endl;

    if(result == message){
        cout << "SUCSESS " << result << endl;
    }

    if(result2 != message){
        cout << "Adversary Failed to open" << endl;
    }else{
        cout << "Adversary SHOULD NOT HAVE THIS " << result2 << endl;
    }

    element_clear(public_key);
    element_clear(private_key);
}