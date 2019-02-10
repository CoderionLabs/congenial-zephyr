#pragma once

#define SIZE 100

#include <gmpxx.h>
#include <iostream>
#include <fstream>
#include <cassert>
#include <utility>
#include <string>
#include <ibe/bf_4_1.hpp>
#include <sodium/crypto_secretbox.h>
#include <sodium/randombytes.h>
#include <sodium/crypto_hash_sha256.h>
#include <cereal/types/string.hpp>
#include <cereal/archives/binary.hpp>

extern "C"{
    #include <sibe/ibe.h>
    #include <sibe/ibe_progs.h>
}


struct contents{
    std::string nonce;
    std::string ciphertext;
    std::string u;

    template<class Archive>
    void serialize(Archive & archive)
    {
        archive(nonce, ciphertext, u); 
    }
};
typedef struct contents contents;

class PKG
{
public:
    params_t params; // public
    byte_string_t master; //unknown to user
    void setup();
    void extract(std::string id, byte_string_t key);
};

void convert_to_encrypt(byte_string_t x, unsigned char* ur);
std::string decrypt(std::string cipher, byte_string_t key, params_t pars);
std::string encrypt(std::string id, params_t pars, std::string msg);