#include "pkg.hpp"

void PKG::setup(int rbits, int qbits){
    setup_sys(rbits, qbits, this->P, this->Ppub, this->pairing, this->masterkey);
    std::cout << "System Parameters Have been setup" << std::endl;
}

void PKG::extract(element_t public_key_r, element_t private_key_r, std::string id){
    // TODO: Authentication code sent to email
    element_init_G1(public_key_r, this->pairing);
    element_init_G1(private_key_r, this->pairing);
    char* ID = const_cast<char*>(id.c_str());
    get_private_key(ID, pairing, masterkey, private_key_r);
    get_public_key(ID, pairing, public_key_r);
}

std::string encrypt(std::string msg, std::string id, element_t P, element_t Ppub, pairing_t pairing){
    //TODO: send U,V,nonce, and ciphertext to the reciever
    element_t U;
    char xor_result[SIZE];
    char shamessage[SIZE];                                 // Sha1 Hash    
    sha_fun(const_cast<char*>(msg.c_str()), shamessage);   //Get the message digest
    
    element_init_G1(U, pairing);
    encryption(shamessage, const_cast<char*>(id.c_str()),P, Ppub, U, xor_result, pairing);

    unsigned char nonce[crypto_secretbox_NONCEBYTES];
    unsigned char ciphertext[crypto_secretbox_MACBYTES  + sizeof(shamessage)];
    randombytes_buf(nonce, sizeof(nonce));
    crypto_secretbox_easy(ciphertext, reinterpret_cast<const unsigned char*>(msg.c_str()),
        sizeof(msg), nonce, reinterpret_cast<const unsigned char*>(shamessage));
    
    // Make a string
    unsigned char ubytes[element_length_in_bytes(U)];
    element_to_bytes(ubytes, U);
    std::cout << ubytes << std::endl;

    std::string u = reinterpret_cast<char*>(ubytes);
    std::string v = std::move(xor_result);
    std::string n = reinterpret_cast<char*>(nonce);
    std::string ciphertextstr = reinterpret_cast<char*>(ciphertext);

    contents c{u,v,n,ciphertextstr};
    //Serialize
    std::stringstream ss; 

    {
        // Create an output archive
        cereal::BinaryOutputArchive oarchive(ss);

        oarchive(c); // Write the data to the archive
    } 
    element_clear(U);
    element_t x;
    element_from_bytes(x, ubytes);
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

    std::cout << c.u << std::endl;
    element_t U;
    unsigned char* tmp = reinterpret_cast<unsigned char*>(reinterpret_cast<char*>(const_cast<char*>(c.u.c_str())));
    element_from_bytes(U, tmp);

    decryption(private_key, pairing, U, const_cast<char*>(c.v.c_str()), xor_result_receiver);

    unsigned char* decrypted;
    if (crypto_secretbox_open_easy(decrypted, reinterpret_cast<const unsigned char*>(c.ciphertext.c_str()), sizeof(c),
        reinterpret_cast<const unsigned char*>(c.nonce.c_str()), reinterpret_cast<const unsigned char*>(xor_result_receiver)) != 0) {
        std::cout << "Failed" << std::endl;
    }
    return reinterpret_cast<char*>(decrypted);
}