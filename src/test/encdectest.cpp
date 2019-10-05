#include <iostream>
#include <vector>
#include <cstring>
#include <string>
#include <memory>
#include <array>
#include <stdio.h> 
#include <sodiumpp/common.h>
#include <sodiumpp/helpers.h>
#include <sodiumpp/secretbox.h>

#define siz(a) strlen((char *) a)

using namespace std;
typedef unsigned char cryptoT;

//FIXME: All of this junk works
// the problem is the cryptolenghts were wrong
// also string conversion is wacky

unsigned char* 
string_to_unsigned_char(std::string s){
    auto x = s.c_str();
    unsigned char tmp[s.length()];
    std::copy(x,x + sizeof(x), tmp);
    cout << tmp << endl;
    return tmp;
}

// Create an onion layer
cryptoT* 
encryptN(cryptoT *msg, cryptoT pub_keys[][32], int size, vector<int> &decsizes){
    cryptoT *tmpmsg = (cryptoT*) malloc(siz(msg));
    memcpy(tmpmsg,msg,siz(msg));

    cout << "ME" << tmpmsg << endl;

    for(int i = 0; i < size; i++){
        cout << i << endl;
        int CIPHERTEXT_LEN = siz(tmpmsg) + crypto_box_SEALBYTES;
        decsizes.push_back(siz(tmpmsg));
        cryptoT ciphertext[CIPHERTEXT_LEN];
        crypto_box_seal(ciphertext, tmpmsg, siz(tmpmsg),pub_keys[i]);
        //free(tmpmsg);
        tmpmsg = (cryptoT*) malloc(sizeof(ciphertext));
        memcpy(tmpmsg,ciphertext, sizeof(ciphertext));
        cout << "ENC" << tmpmsg << endl;
    }
    cout << "WORKS" << endl;
    return tmpmsg;
}

// decrypt an onion layer
// num should be equal to i + 1
cryptoT* 
decryptN(cryptoT *ciphertext, cryptoT pub_keys[][32], cryptoT pri_keys[][32],
         int size, vector<int> decsizes){
    int j = decsizes.size() -1;
    cryptoT *cipher = (cryptoT*) malloc(siz(ciphertext));
    memcpy(cipher,ciphertext, siz(ciphertext));

    int g = 0;
    for(int i = size-1; i > -1; i--){
        int DECRYPTLEN = decsizes[j - g];
        cryptoT decrypted[DECRYPTLEN];
        cout << "LEN " << DECRYPTLEN << endl;
        //FIXME: Crypto box not working
        crypto_box_seal_open(decrypted, cipher, siz(cipher), pub_keys[i], pri_keys[i]);
        cout << "Works" << endl;
        free(cipher);
        cipher = (cryptoT*) malloc(sizeof(decrypted));
        
        memcpy(cipher, decrypted, sizeof(decrypted));
        cout << "I GOT " << decrypted << endl;
        cout << i << endl;
        if(i == 0){
            return decrypted;
        }
        g++;
    }
    
    return ciphertext;
}

int 
main(){
    //TODO: Enc and Dec test
    int N = 2;
    int M = 32;

    // TODO: Fix this
    string tmp;
    cout << "Enter Text" << endl;
    cin >> tmp;

    sodium::bytes msg{tmp.cbegin(),tmp.cend()};
    // Initilize rows
    cryptoT public_keys[2][32];
    cryptoT private_keys[2][32];

    for(int i = 0; i < 2; i++){
        cryptoT recipient_pk[crypto_box_PUBLICKEYBYTES];
        cryptoT recipient_sk[crypto_box_SECRETKEYBYTES];
        crypto_box_keypair(recipient_pk, recipient_sk);
        memcpy(public_keys[i], recipient_pk, sizeof(recipient_pk));
        memcpy(private_keys[i], recipient_sk, sizeof(recipient_sk));
    }

    std::vector<int> decsizes;
    cryptoT* x = encryptN(&msg[0], public_keys, 2, decsizes);
    cout << x << endl;
    cryptoT* y = decryptN(x, public_keys, private_keys, 2, decsizes);
    std::vector<int> v(y, y + sizeof(y) / sizeof(y[0]));
    cout << sodium::bin2hex(v) << endl;

	return 0;
}