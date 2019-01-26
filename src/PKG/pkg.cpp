#include "pkg.hpp"

void PKG::setup(int rbits, int qbits){
    setup_sys(rbits, qbits, this->P, this->Ppub, this->pairing, this->masterkey);
    std::cout << "System Parameters Have been setup" << std::endl;
}

void PKG::extract(element_t public_key_r, element_t private_key_r, std::string id){
    // TODO: Authentication code sent to email
    element_init_G1(public_key_r, this->pairing);
    element_init_G1(private_key_r, this->pairing);
    get_private_key(&id[0], this->pairing, this->masterkey, public_key_r);
    get_public_key(&id[0], this->pairing, private_key_r);
}

std::string encrypt(std::string msg, std::string id, element_t P, element_t Ppub, pairing_t pairing){
    //TODO: send U,V,nonce, and ciphertext to the reciever
    element_t U;
    char xor_result[SIZE];
    char shamessage[SIZE];                                 // Sha1 Hash    
    sha_fun(&msg[0], shamessage);   //Get the message digest
    
    element_init_G1(U, pairing);
    encryption(shamessage, &id[0],P, Ppub, U, xor_result, pairing);

    unsigned char nonce[crypto_secretbox_NONCEBYTES];
    unsigned char ciphertext[crypto_secretbox_MACBYTES  + msg.length()];
    randombytes_buf(nonce, sizeof(nonce));
    crypto_secretbox_easy(ciphertext, reinterpret_cast<unsigned char*>(&msg[0]),
        msg.length(), nonce, reinterpret_cast<unsigned char*>(shamessage));
 
    // Make a string
    unsigned char ubytes[element_length_in_bytes(U)];
    element_to_bytes(ubytes, U);

    std::string u(reinterpret_cast<char*>(ubytes));
    std::string v(xor_result);
    std::string n(reinterpret_cast<char*>(nonce));
    std::string ciphertextstr(reinterpret_cast<char*>(ciphertext));

    contents c{u,v,n,ciphertextstr};
    //Serialize
    std::stringstream ss; 

    {
        // Create an output archive
        cereal::BinaryOutputArchive oarchive(ss);

        oarchive(c); // Write the data to the archive
    }


    //assert(x == U);
    std::cout << "ENCRYPTOIN PASSED" << std::endl;
    return ss.str();
}

std::string decrypt(element_t private_key, std::string content, pairing_t pairing){
    char xor_result_receiver[SIZE];
    
    std::stringstream ss;
    ss.write(content.c_str(),content.size());
    contents c;
    {
        cereal::BinaryInputArchive iarchive(ss);
        iarchive(c); // Read the data from the archive
    }

    element_t U;
    unsigned char* tmp = reinterpret_cast<unsigned char*>(&c.u[0]);

    element_init_G1(U, pairing);
    element_from_bytes(U, tmp);

    char xord[c.v.length()];
    
    decryption(private_key, pairing, U, &c.v[0], xor_result_receiver);

    unsigned char decrypted[52]; //TODO: Change this later
    if (crypto_secretbox_open_easy(decrypted, reinterpret_cast<unsigned char*>(&c.ciphertext[0]), c.ciphertext.length(),
        reinterpret_cast<unsigned char*>(&c.nonce[0]), reinterpret_cast<unsigned char*>(xor_result_receiver)) != 0) {
        std::cout << "Failed" << std::endl;
    }
    return reinterpret_cast<char*>(decrypted);
}