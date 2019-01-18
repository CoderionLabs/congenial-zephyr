#pragma once

#define SIZE 100

#include <iostream>
#include <fstream>
#include <ibe/bf_4_1.hpp>
#include <sodium/crypto_secretbox.h>
#include <sodium/randombytes.h>
#include "json.hpp"

struct contents{
    std::string u;
    std::string v;
    std::string nonce;
    std::string ciphertext;
};
typedef struct contents contents;

class PKG
{
private:
   int n = 0;
   element_t P, Ppub, masterkey; // Master public and private keys
   pairing_t pairing;   //The pair of bilinear map
public:
    void setup(int rbits, int qbits);
    void extract(element_t Qid, element_t Sid, std::string id);
    std::string decrypt(element_t private_key, std::string content);
    std::string encrypt(std::string msg, std::string id, char* xor_result);
};


