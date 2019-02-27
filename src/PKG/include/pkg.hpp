#pragma once

#define SIZE 100

#include <gmpxx.h>
#include <iostream>
#include <fstream>
#include <cassert>
#include <utility>
#include <string>
#include <ibe/bf_4_1.hpp>
#include <algorithm>
#include <iterator>
#include <bitset>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/lexical_cast.hpp>
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
    std::string size;
    std::string ciphersize;

    template<class Archive>
    void serialize(Archive & archive)
    {
        archive(nonce, ciphertext, u, size, ciphersize); 
    }
};
typedef struct contents contents;

struct bytestring_wrap{
    std::string data;
    std::string size;

    template<class Archive>
    void serialize(Archive & archive)
    {
        archive(data,size); 
    }
};
typedef struct bytestring_wrap bytestring_wrap;

class PKG
{
public:
    params_t params; // public
    byte_string_t master; //unknown to user
    ~PKG();
    void setup(std::string system);
    std::string serialize_params(params_t p);
    std::string serialize_bytestring(byte_string_t b);
    void extract(std::string id, byte_string_t key);
};

void convert_to_encrypt(byte_string_t x, unsigned char* ur);
std::string pkg_decrypt(std::string cipher, byte_string_t key, params_t pars);
std::string pkg_encrypt(std::string id, params_t pars, std::string msg);