#include <iostream>
#include <vector>
#include <cstring>
#include <string>
#include <array>
#include <stdio.h> 
#include <sodium.h>

#define siz(a) strlen((char *) a)

// #define msg  (const unsigned char *) "ignoring return value of ‘int crypto_box_seal_open(unsigned char*, const unsigned char*, long long unsigned int, const unsigned char*, const unsigned char*)’, declared with attribute warn"

using namespace std;
typedef unsigned char cryptoT;

//FIXME: All of this junk works
// the problem is the cryptolenghts were wrong
// also string conversion is wacky

unsigned char* string_to_unsigned_char(std::string s){
    auto x = s.c_str();
    unsigned char tmp[s.length()];
    std::copy(x,x + sizeof(x), tmp);
    cout << tmp << endl;
    return tmp;
}

// Create an onion layer
cryptoT* encryptN(cryptoT *ciphertext, cryptoT pub_keys[][32],
    int num, int i, std::vector<int>& decsizes){
    if(i == num){
        cout << "WORKS" << endl;
        return ciphertext;
    }
    cout << ciphertext << endl;
    cout << siz(ciphertext) << endl;
    decsizes.push_back(siz(ciphertext));
    int CIPHTERTEXT_LEN = siz(ciphertext) + crypto_box_SEALBYTES;
    // cout << siz(ciphertext) << endl;
    cryptoT encmsg[CIPHTERTEXT_LEN];
    cout << i << endl;
    crypto_box_seal(encmsg, ciphertext, siz(ciphertext), pub_keys[0]);
    cout << encmsg << "CAKE" << endl;
    encryptN(encmsg,pub_keys, num, ++i, decsizes);
}

// decrypt an onion layer
// num should be equal to i + 1
cryptoT* decryptN(cryptoT *ciphertext, cryptoT pub_keys[][32],
    cryptoT pri_keys[][32], int i, std::vector<int> decsizes){
    if(i == -1){return ciphertext;}
    cryptoT decrypted[1000];
    // cout << decsizes[i] << endl;
    auto size = siz(ciphertext);
    if(crypto_box_seal_open(decrypted, ciphertext, size,
    pub_keys[i], pri_keys[i]) !=0){
        cerr << "Failed" << endl;
        return nullptr;
    }
    cout << pub_keys[i] << "CAKEEND"<< endl;
    decryptN(decrypted, pub_keys, pri_keys,--i, decsizes);
}

int main(){
    //TODO: Enc and Dec test
    int N = 5;
    int M = 32;

    // TODO: Fix this
    string tmp;
    cout << "Enter Text" << endl;
    cin >> tmp;
    unsigned char msg[tmp.length()];
    cout << "Can you enter it again it must be the exact same." << endl;
    scanf("%s", msg);
    // Initilize rows
    cryptoT public_keys[1][32];
    cryptoT private_keys[1][32];

    for(int i = 0; i < 1; i++){
        cryptoT recipient_pk[crypto_box_PUBLICKEYBYTES];
        cryptoT recipient_sk[crypto_box_SECRETKEYBYTES];
        crypto_box_keypair(recipient_pk, recipient_sk);
        memcpy(public_keys[i], recipient_pk, sizeof(recipient_pk));
        memcpy(private_keys[i], recipient_sk, sizeof(recipient_sk));
    }

    std::vector<int> decsizes;
    auto x = encryptN(msg, public_keys, 1, 0, decsizes);
    auto y = decryptN(x, public_keys, private_keys, 1, 0, decsizes);
    cout << y << endl;
   
    // // Encrypt the messages
    // int size1 = 192;
    // int CIPHERTEXT_LEN = size1 + crypto_box_SEALBYTES;
    // cryptoT ciphertext[CIPHERTEXT_LEN];
    // crypto_box_seal(ciphertext, msg,
    // size1, public_keys[0]);

    // int size2 = CIPHERTEXT_LEN;
    // int CIPHERTEXT_LEN2 = size2 + crypto_box_SEALBYTES;
    // cryptoT ciphertext2[CIPHERTEXT_LEN2];
    // crypto_box_seal(ciphertext2, ciphertext,
    // size2, public_keys[1]);



    // // Decrypt the messages
    // cryptoT decrypted[size2];
    // crypto_box_seal_open(decrypted, ciphertext2, CIPHERTEXT_LEN2,
    // public_keys[1], private_keys[1]);

    // cryptoT decrypted2[size1];
    // crypto_box_seal_open(decrypted2, decrypted, CIPHERTEXT_LEN,
    // public_keys[0], private_keys[0]);
    
    // cout << decrypted2 << endl;
    // // crypto_box_seal(ciphertext, MESSAGE, MESSAGE_LEN, recipient_pk);
	return 0;
}