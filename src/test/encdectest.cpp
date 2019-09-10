#include <iostream>
#include <vector>
#include <cstring>
#include <string>
#include <array>
#include <stdio.h> 
#include <sodium.h>

#define msg  (const unsigned char *) "ignoring return value of ‘int crypto_box_seal_open(unsigned char*, const unsigned char*, long long unsigned int, const unsigned char*, const unsigned char*)’, declared with attribute warn"

using namespace std;
typedef unsigned char cryptoT;

//FIXME: All of this junk works
// the problem is the cryptolenghts were wrong
// also string conversion is wacky

int main(){
    //TODO: Enc and Dec test
    int N = 5;
    int M = 32;

    // Initilize rows
    cryptoT public_keys[5][32];
    cryptoT private_keys[5][32];
    int sizes[5];

    for(int i = 0; i < 5; i++){
        cryptoT recipient_pk[crypto_box_PUBLICKEYBYTES];
        cryptoT recipient_sk[crypto_box_SECRETKEYBYTES];
        crypto_box_keypair(recipient_pk, recipient_sk);
        memcpy(public_keys[i], recipient_pk, sizeof(recipient_pk));
        memcpy(private_keys[i], recipient_sk, sizeof(recipient_sk));
    }
   
    cout << "THISIS " << sizeof(public_keys)/32 << "\n";

    // Encrypt the messages
    int size1 = 192;
    int CIPHERTEXT_LEN = size1 + crypto_box_SEALBYTES;
    cryptoT ciphertext[CIPHERTEXT_LEN];
    crypto_box_seal(ciphertext, msg,
    size1, public_keys[0]);

    int size2 = CIPHERTEXT_LEN;
    int CIPHERTEXT_LEN2 = size2 + crypto_box_SEALBYTES;
    cryptoT ciphertext2[CIPHERTEXT_LEN2];
    crypto_box_seal(ciphertext2, ciphertext,
    size2, public_keys[1]);



    // Decrypt the messages
    cryptoT decrypted[size2];
    crypto_box_seal_open(decrypted, ciphertext2, CIPHERTEXT_LEN2,
    public_keys[1], private_keys[1]);

    cryptoT decrypted2[size1];
    crypto_box_seal_open(decrypted2, decrypted, CIPHERTEXT_LEN,
    public_keys[0], private_keys[0]);
    
    cout << decrypted2 << endl;
    // crypto_box_seal(ciphertext, MESSAGE, MESSAGE_LEN, recipient_pk);
	return 0;
}