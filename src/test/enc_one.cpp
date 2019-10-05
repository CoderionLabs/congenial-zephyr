#include <iostream>
#include <vector>
#include <cstring>
#include <string>
#include <array>
#include <stdio.h> 
#include <sodium.h>
#define msg  (const unsigned char *) "TEST APPLE APLE apple"


using namespace std;
typedef unsigned char cryptoT;

int main(){
    //TODO: Enc and Dec test
    string message;
    cout << "Enter Message" << endl;
    cin >> message;
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
    int size = 22;
    int CIPHERTEXT_LEN = size + crypto_box_SEALBYTES;
    cryptoT ciphertext[CIPHERTEXT_LEN];
    crypto_box_seal(ciphertext, msg,
    size, public_keys[1]);



    // Decrypt the messages
    cryptoT decrypted[size];
    crypto_box_seal_open(decrypted, ciphertext, CIPHERTEXT_LEN,
    public_keys[1], private_keys[1]);
    
    cout << decrypted << endl;
    // crypto_box_seal(ciphertext, MESSAGE, MESSAGE_LEN, recipient_pk);
	return 0;
}