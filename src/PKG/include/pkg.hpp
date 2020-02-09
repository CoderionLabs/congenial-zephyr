/*
 * Copyright (c) 2020 Mutex Unlocked
 * Author: Friedrich Doku
 * -----
 * Last Modified: Friday May 24th 2020 11:54:38 am
 * -----
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *   This program is distributed in the hope that it will be useful
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <https://www.gnu.org/licenses/>.
 */



#pragma once

#define SIZE 100

#include <gmpxx.h>
#include <iostream>
#include <fstream>
#include <cassert>
#include <utility>
#include <string>
#include <algorithm>
#include <vector>
#include <iterator>
#include <bitset>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/lexical_cast.hpp>
#include <sodium/crypto_secretbox.h>
#include <sodium/randombytes.h>
#include <sodium/crypto_hash_sha256.h>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/archives/portable_binary.hpp>

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
    std::vector<unsigned char> data;
    int size;

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
    std::string serialize_params();
    std::string serialize_bytestring(byte_string_t b);
    void extract(std::string id, byte_string_t key);
};

void convert_to_encrypt(byte_string_t x, unsigned char* ur);
std::string pkg_decrypt(std::string cipher, byte_string_t key, params_t pars);
std::string pkg_encrypt(std::string id, params_t pars, std::string msg);
void deserialize_bytestring(std::string p, byte_string_t result);
void deserialize_params(std::string in, params_t p);