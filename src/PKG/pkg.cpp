#include "pkg.hpp"

void PKG::setup(){
     //initialize IBE library
    IBE_init();

    //generate new system parameters:
    //128-bit prime, 64-bit subgroup size

    IBE_setup(params, master, 512, 160, "dokuenterprise");
}

void PKG::extract(std::string id, byte_string_t key){
   // TODO: Authenticate
   byte_string_t x;
   IBE_extract(key, this->master, id.c_str(), this->params);
}

std::string encrypt(std::string id, params_t pars, std::string msg){
    byte_string_t secret;
    byte_string_t U;

    IBE_KEM_encrypt(secret, U, &id[0], pars);

    unsigned char* dd = (unsigned char*) malloc(sizeof(unsigned char) * 100);
    convert_to_encrypt(secret, dd);

    unsigned char nonce[crypto_secretbox_NONCEBYTES];
    unsigned char ciphertext[crypto_secretbox_MACBYTES + sizeof(msg)];
    unsigned char out[crypto_hash_sha256_BYTES];
    crypto_hash_sha256(out, dd, sizeof(dd));
    
    //cout << out << endl;

    randombytes_buf(nonce, sizeof nonce);
    crypto_secretbox_easy(ciphertext, reinterpret_cast<const unsigned char*>(msg.c_str()),
         msg.size(), nonce, out);

    // Put nonce, ciphertext, and U into the envolope

    std::string nonce_str = reinterpret_cast<char*>(nonce);
    std::string ciphertext_str = reinterpret_cast<char*>(ciphertext);
    std::string U_str = reinterpret_cast<char*>(U->data);
    // Length of U is sizeof(U->data)/sizeof(unsigned char);

    contents c{nonce_str, ciphertext_str, U_str};
    //Serialize
    std::stringstream ss; 

    {
        // Create an output archive
        cereal::BinaryOutputArchive oarchive(ss);

        oarchive(c); // Write the data to the archive
    }

    return ss.str();
}

std::string decrypt(std::string cipher, byte_string_t key, params_t pars){
    std::stringstream ss;
    ss.write(cipher.c_str(),cipher.size());
    contents c;
    {
        cereal::BinaryInputArchive iarchive(ss);
        iarchive(c); // Read the data from the archive
    }
    
    byte_string_t U;
    
    unsigned char tmp[sizeof(c.u)];
    for(int i = 0; i < c.u.size(); i++){
        tmp[i] = c.u[i];
    }

    U->data = tmp;
    U->len = sizeof(U->data)/sizeof(unsigned char);
    U->origlen = sizeof(U->data)/sizeof(unsigned char);


    byte_string_t secret;
    IBE_KEM_decrypt(secret, U, key, pars);


    unsigned char* dd2 = (unsigned char*) malloc(sizeof(unsigned char) * 100);
    convert_to_encrypt(secret, dd2);
    // Test Recipent
    unsigned char out2[crypto_hash_sha256_BYTES];
    crypto_hash_sha256(out2, dd2, sizeof(dd2));
    //cout << out2 << endl;

    unsigned char decrypted[100000]; //TODO: hack fix later
    if (crypto_secretbox_open_easy(decrypted, reinterpret_cast<unsigned char*>(&c.u[0]),
     c.u.size(),  reinterpret_cast<unsigned char*>(&c.nonce[0]), out2) != 0) {
        printf("Failed to decrypted message\n");
    }else{
        std::cout << decrypted << std::endl;
    }

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