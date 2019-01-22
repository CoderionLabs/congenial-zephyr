#pragma once

#define SIZE 100

#include <iostream>
#include <fstream>
#include <cassert>
#include <utility>
#include <ibe/bf_4_1.hpp>
#include <sodium/crypto_secretbox.h>
#include <sodium/randombytes.h>
#include <cereal/types/string.hpp>

#include <cereal/archives/binary.hpp>

struct contents{
    std::string u;
    std::string v;
    std::string nonce;
    std::string ciphertext;

    template<class Archive>
    void serialize(Archive & archive)
    {
        archive(u,v,nonce,ciphertext); 
    }
};
typedef struct contents contents;

class PKG
{
private:
   int n = 0;
public:
    element_t P, Ppub, masterkey; // Master public and private keys
    pairing_t pairing;            //The pair of bilinear map
    void setup(int rbits, int qbits);
    void extract(element_t public_key_r, element_t private_key_r, std::string id);
};

std::string decrypt(element_t private_key, std::string content, pairing_t pairing);
std::string encrypt(std::string msg, std::string id, element_t P, element_t Ppub, pairing_t pairing);