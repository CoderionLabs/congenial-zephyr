#include "pkg.hpp"

void PKG::setup(std::string system){
     //initialize IBE library
    IBE_init();

    //generate new system parameters:
    //128-bit prime, 64-bit subgroup size

    IBE_setup(this->params, this->master, 512, 160, &system[0]);
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
        cereal::BinaryOutputArchive oarchive(ss);

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
        cereal::BinaryInputArchive iarchive(ss);
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
