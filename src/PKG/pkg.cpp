#include "pkg.hpp"

using json = nlohmann::json;

void PKG::setup(int rbits, int qbits){
    setup_sys(rbits, qbits, this->P, this->Ppub, this->pairing, this->masterkey);
    std::cout << "System Parameters Have been setup" << std::endl;
}

void PKG::extract(element_t public_key, element_t private_key, std::string id){
    // TODO: Authentication code sent to email
    element_init_G1(public_key, this->pairing);
    element_init_G1(private_key, this->pairing);
    char* ID = const_cast<char*>(id.c_str());
    get_private_key(ID, pairing, masterkey, private_key);
    get_public_key(ID, pairing, public_key);
}

std::string PKG::encrypt(std::string msg, std::string id, char* xor_result){
    //TODO: send U,V,nonce, and ciphertext to the reciever
    element_t U;
    char shamessage[SIZE];                                 // Sha1 Hash    
    sha_fun(const_cast<char*>(msg.c_str()), shamessage);   //Get the message digest
    
    element_init_G1(U, this->pairing);
    encryption(const_cast<char*>(msg.c_str()), const_cast<char*>(id.c_str()),
     P, Ppub, U, xor_result, this->pairing);


    unsigned char nonce[crypto_secretbox_NONCEBYTES];
    unsigned char ciphertext[crypto_secretbox_MACBYTES  + sizeof(shamessage)];
    randombytes_buf(nonce, sizeof(nonce));
    crypto_secretbox_easy(ciphertext, reinterpret_cast<const unsigned char*>(msg.c_str()),
        sizeof(msg), nonce, reinterpret_cast<const unsigned char*>(shamessage));

    // Make a string
    unsigned char* ubytes;
    std::string u = reinterpret_cast<char*>(element_to_bytes(ubytes, U));
    std::string v = std::move(xor_result);
    std::string n = reinterpret_cast<char*>(nonce);
    std::string ciphertextstr = reinterpret_cast<char*>(ciphertext);

    contents c{u,v,n,ciphertextstr};
    //Serialize
    json j;
    j["u"] = u;
    j["v"] = v;
    j["n"] = n;
    j["c"] = ciphertextstr;

    return j.dump();
}

std::string PKG::decrypt(element_t private_key, std::string content){
    char xor_result_receiver[SIZE];
    json j = json::parse(content);
    std::string u = j["u"];
    std::string v = j["v"];
    std::string n = j["n"];
    std::string c = j["c"];

    element_t U;
    element_from_bytes(U, const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(u.c_str())));

    decryption(private_key, this->pairing, U, const_cast<char*>(v.c_str()), xor_result_receiver);

    unsigned char* decrypted;
    if (crypto_secretbox_open_easy(decrypted, reinterpret_cast<const unsigned char*>(c.c_str()), sizeof(c),
        reinterpret_cast<const unsigned char*>(n.c_str()), reinterpret_cast<const unsigned char*>(xor_result_receiver)) != 0) {
        std::cout << "Failed" << std::endl;
    }
    return reinterpret_cast<char*>(decrypted);
}