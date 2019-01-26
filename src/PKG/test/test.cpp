// Just a test to prove correctness
#define MESSAGE ((const unsigned char *) "The")
#define MESSAGE_LEN 3
#define CIPHERTEXT_LEN (crypto_secretbox_MACBYTES + MESSAGE_LEN)

#include <iostream>
#include <cassert>
#include <zepher/pkg.hpp>

int main(){
    using namespace std;

    /*unsigned char key[crypto_secretbox_KEYBYTES];
    unsigned char nonce[crypto_secretbox_NONCEBYTES];
    unsigned char ciphertext[CIPHERTEXT_LEN];

    crypto_secretbox_keygen(key);
    randombytes_buf(nonce, sizeof nonce);
    crypto_secretbox_easy(ciphertext, MESSAGE, MESSAGE_LEN, nonce, key);

    unsigned char decrypted[MESSAGE_LEN];
    if (crypto_secretbox_open_easy(decrypted, ciphertext, CIPHERTEXT_LEN, nonce, key) != 0) {
        std::cout << "Failed" << std::endl;
    }
    std::cout << decrypted << std::endl;*/
    std::string message = "You can't just stop here you still have more ahead.";
    std::string id = "friedrichdoku";

    PKG p;
    p.setup(5,5);
    element_t public_key, private_key;
    p.extract(public_key, private_key, id);

    auto str = encrypt(message,id,p.P, p.Ppub,p.pairing);
    auto result = decrypt(private_key,str,p.pairing);

    cout << result << endl;
    cout << message << endl;
}