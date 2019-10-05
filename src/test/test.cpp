#include <iostream>
#include <vector>
#include <cstring>
#include <string>
#include <array>
#include <stdio.h> 
#include <sodium.h>

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
    cryptoT **public_keys = (cryptoT **)malloc(sizeof(cryptoT *)*N);
    cryptoT **private_keys = (cryptoT **)malloc(sizeof(cryptoT *)*N);
    int sizes[5];
    
    // Create colums for the rows
    for(int i = 0; i < N; i++) {
        public_keys[i] = (cryptoT *)malloc(sizeof(cryptoT)*M);
        private_keys[i] = (cryptoT *)malloc(sizeof(cryptoT)*M);
    }

    for(int i = 0; i < N; i++){
        for(int j = 0; j < M; j++){
            public_keys[i][j] = 0;
            private_keys[i][j] = 0;
        }
    }


    for(int i = 0; i < sizeof(public_keys[0]); i++){
        cryptoT recipient_pk[crypto_box_PUBLICKEYBYTES];
        cryptoT recipient_sk[crypto_box_SECRETKEYBYTES];
        crypto_box_keypair(recipient_pk, recipient_sk);
        public_keys[i] = recipient_pk;
        private_keys[i] = recipient_sk;
    }
    
   
    cout << "THISIS " << sizeof(public_keys[0]) << "\n";
    // Encrypt the messages
    cryptoT* ciphertext;
    for(int i = 0; i < sizeof(public_keys[0]); i++){
        cout << i << endl;
        int CIPHERTEXT_LEN = message.size() + crypto_box_SEALBYTES;
        ciphertext = (cryptoT *)malloc(sizeof(cryptoT) * CIPHERTEXT_LEN);
        bzero(ciphertext, sizeof(ciphertext));
        crypto_box_seal(ciphertext, reinterpret_cast<cryptoT*>(&message[0]),
         message.length(), public_keys[i]);
         sizes[i] = message.length();
         message = reinterpret_cast<char*>(ciphertext);
         cout << ciphertext << endl;
         cout << "****************************************" << endl;
    }

    // Decrypt the messages
    cryptoT* decrypted;
    for(int i = 4; i > -1; i--){
        decrypted = (cryptoT *)malloc(sizeof(cryptoT) * sizes[i]);
        crypto_box_seal_open(decrypted, ciphertext, sizeof(ciphertext),
                         public_keys[i], private_keys[i]);
        ciphertext = (cryptoT *)malloc(sizeof(decrypted));
        memcpy(ciphertext, decrypted, sizeof(decrypted));
        cout << decrypted << endl;
    }

   // crypto_box_seal(ciphertext, MESSAGE, MESSAGE_LEN, recipient_pk);
	return 0;
}