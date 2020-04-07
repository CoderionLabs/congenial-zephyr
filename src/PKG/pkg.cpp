/*
 * Copyright (c) 2020 Mutex Unlocked
 * Author: Friedrich Doku
 * -----
 * Last Modified: Sunday April 21st 2020 1:25:50 pm
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

#include "pkg.hpp"

void PKG::setup(std::string system){
     //initialize IBE library
    IBE_init();

    //generate new system parameters:
    //128-bit prime, 64-bit subgroup size

    IBE_setup(this->params, this->master, 512, 160, &system[0]);
    /*FILE *fp;
    fopen("file.txt", "w+");
    params_out(fp, this->params);*/
    //fclose(fp);
    std::cout << "SETUP DONE" << std::endl;
}

PKG::~PKG(){
    params_clear(this->params);
    byte_string_clear(this->master);
    IBE_clear();
}

void PKG::extract(std::string id, byte_string_t key){
   // TODO: Authenticate
   IBE_extract(key, this->master, &id[0], this->params);
   std::cout << "EXTRACT DONE" << std::endl;
}

void deserialize_params(std::string in, params_t p){
    byte_string_t tmp;
    deserialize_bytestring(in, tmp);
    std::cout << "START" << std::endl;
    IBE_deserialize_params(p, tmp);
    std::cout << "END" << std::endl;
}

std::string PKG::serialize_params(){
    byte_string_t b;
    IBE_serialize_params(b,this->params);
    auto s = serialize_bytestring(b);
    return s;
}

void deserialize_bytestring(std::string p, byte_string_t result){
    std::stringstream ss;
    ss.write(p.c_str(), p.size());
    bytestring_wrap c;
    {
        cereal::PortableBinaryInputArchive iarchive(ss);
        iarchive(c); // Read the data from the archive
    }
    byte_string_init(result, c.size);
    for(int i = 0; i < c.size; i++){
        result->data[i] = c.data[i];
    }
}

std::string PKG::serialize_bytestring(byte_string_t b){
    std::vector<unsigned char> data;
    for(int i = 0; i < b->len; i++){
        data.push_back(b->data[i]);
    }
    int size = (b->len);
    
    bytestring_wrap present{data, size};
    //Serialize
    std::stringstream ss;
    {
        // Create an output archive
        cereal::PortableBinaryOutputArchive oarchive(ss);

        oarchive(present); // Write the data to the archive
    }
    return ss.str();
}

std::string pkg_encrypt(std::string id, params_t pars, std::string msg){
    byte_string_t secret;
    byte_string_t U;

    std::cout << pars->id << std::endl;

    IBE_KEM_encrypt(secret, U, &id[0], pars);
    printf("secret =");
    byte_string_printf(secret, " %02X");
    printf("\n");

    //assert(dd[5] == junk[5]);
    unsigned char* dd = (unsigned char*) malloc(sizeof(unsigned char) * 100);
    convert_to_encrypt(secret, dd);
    byte_string_clear(secret);

    auto ciphersize = (crypto_secretbox_MACBYTES) + (msg.length());
    std::cout << "SIZE IS " << ciphersize << std::endl;
    unsigned char ciphertext[ciphersize];
    unsigned char nonce[crypto_secretbox_NONCEBYTES];
    unsigned char out[crypto_hash_sha256_BYTES];
    crypto_hash_sha256(out, dd, sizeof(dd));

    std::cout << dd << std::endl;
    std::cout << "KEY " << out << std::endl;

    randombytes_buf(nonce, sizeof(nonce));

    crypto_secretbox_easy(ciphertext, reinterpret_cast<unsigned char*>(&msg[0]),
        msg.length(), nonce, dd);

    std::cout << "CIPHERTEXT " << ciphertext << std::endl;

    // Put nonce, ciphertext, and U into the envolope

    unsigned char data[U->len];
    for(int i = 0; i < U->len; i++){
        data[i] = U->data[i];
    }

    // Convert to binary
    std::string binary = std::bitset<8>(ciphersize).to_string();

    std::string nonce_str(reinterpret_cast<char*>(nonce));
    std::string ciphertext_str(reinterpret_cast<char*>(ciphertext));
    std::string U_str(data, data + U->len);
    std::string size = std::to_string(U->len);
    std::string ciphersize_str(binary);
    // Length of U is sizeof(U->data)/sizeof(unsigned char);

    contents c{nonce_str, ciphertext_str, U_str, size, ciphersize_str};
    //Serialize
    std::stringstream ss;
    {
        // Create an output archive
        cereal::PortableBinaryOutputArchive oarchive(ss);

        oarchive(c); // Write the data to the archive
    }
    byte_string_clear(U);

    /*std::cout << "LET ME TEST SOMETHING FIRST" << std::endl;
    unsigned long sizel = stoul(ciphersize_str);
    std::cout << "THIS IS IT " <<  sizel << std::endl;*/

    std::cout << "ENCRYPT DONE" << std::endl;
    return ss.str();
}

std::string pkg_decrypt(std::string cipher, byte_string_t key, params_t pars){
    std::stringstream ss;
    ss.write(cipher.c_str(),cipher.size());
    contents c;
    {
        cereal::PortableBinaryInputArchive iarchive(ss);
        iarchive(c); // Read the data from the archive
    }
    auto size = std::bitset<8>(c.ciphersize).to_ulong();;
    std::cout << "THIS IS SIZE " << size << std::endl;
    byte_string_t U;
    byte_string_init(U, std::stoi(c.size));
    U->data = reinterpret_cast<unsigned char*>(&c.u[0]);

    byte_string_t secret;

    IBE_KEM_decrypt(secret, U, key, pars);
    std::cout << "Decrypted died" << std::endl;

    printf("secret =");
    byte_string_printf(secret, " %02X");
    printf("\n");

    unsigned char* dd2 = (unsigned char*) malloc(sizeof(unsigned char) * 100);
    convert_to_encrypt(secret, dd2);

    // Test Recipent
    unsigned char out2[crypto_hash_sha256_BYTES];
    crypto_hash_sha256(out2, dd2, sizeof(dd2));

    std::cout << dd2 << std::endl;
    std::cout << "KEY " << out2 << std::endl;

    unsigned char decrypted[100000]; //TODO: hack fix later;

    auto ciphertmpgo = reinterpret_cast<const unsigned char*>(&c.ciphertext[0]);
    auto noncetmp = reinterpret_cast<const unsigned char*>(&c.nonce[0]);

    std::cout << "CIPHERTEXT " << ciphertmpgo << std::endl;
    //int x = std::stoi(c.ciphersize);
    //std::cout << x << std::endl;
    
    if (crypto_secretbox_open_easy(decrypted, ciphertmpgo, size, noncetmp, dd2) != 0) {
        printf("Failed to decrypted message\n");
    }else{
        std::cout << decrypted << std::endl;
    }

    byte_string_clear(U);
    byte_string_clear(secret);

    std::string result;
    result = reinterpret_cast<char*>(decrypted);

    return result;
}


void convert_to_encrypt(byte_string_t x, unsigned char* ur){
   char s[1000]; std::string result = "";
   for(int i = 0; i < x->len; i++){
        snprintf(s, 1000, "%02X", x->data[i]);
        result += s;
        memset(s, 0, sizeof s);
   }

   for(int i = 0; i < result.size(); i++){
       ur[i] =  result[i];
   }
}
