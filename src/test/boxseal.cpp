#include <sodiumpp/sodiumpp.h>
#include <string>
#include <vector>
#include <iostream>


using namespace sodiumpp;

int main(int argc, const char ** argv) {
    box_secret_key sk_client;
    box_secret_key sk_server;

    std::cout << "Client key: " << sk_client << std::endl;
    std::cout << "Server key: " << sk_server << std::endl;
    std::cout << std::endl;

    // Uses predefined nonce type with 64-bit sequential counter 
    // and constant random bytes for the rest

    boxer<nonce64> client_boxer(sk_server.pk, sk_client);
    unboxer<nonce64> server_unboxer(sk_client.pk, sk_server, client_boxer.get_nonce_constant());

    std::vector<nonce64> nonces;
    std::string x = "Hello DEMS";
    for(int i = 0; i < 1000; i++){
        nonce64 tmp;
        encoded_bytes boxed = client_boxer.box(x, tmp);
        nonces.push_back(tmp);
        x = boxed.bytes;
    }

    // Now unbox this shit
    
    std::string unboxed;
    for(int i = 0; i < 1000; i++){
        
        if(i == 0){
            encoded_bytes j(x, encoding::binary); 
            unboxed = server_unboxer.unbox(j, nonces[nonces.size() -1]);
            nonces.pop_back();
        }else{
             encoded_bytes v(unboxed, encoding::binary);
        unboxed = server_unboxer.unbox(v, nonces[nonces.size() -1]);
        nonces.pop_back();
        std::cout << "Unboxed message: " << i << "  " << unboxed << std::endl;
        } 

    }
    // nonce64 used_n;
    // nonce64 used_n2;
    // encoded_bytes boxed = client_boxer.box("Hello, world!\n", used_n);
    // encoded_bytes boxed2= client_boxer.box(boxed.bytes,used_n2);

    // std::cout << "Nonce (hex): " << used_n2.get(encoding::hex).bytes << std::endl;
    // std::cout << "Boxed message (z85): " << boxed2.to(encoding::z85).bytes << std::endl;
    // Nonce is passed explicitly here, but will also be increased automatically
    // if unboxing happens in the same order as boxing.
    // In a real application this nonce would be passed along with the boxed message.
    // std::string unboxed = server_unboxer.unbox(boxed2, used_n2);
    // encoded_bytes j(unboxed,encoding::binary);

    // unboxed = server_unboxer.unbox(j, used_n);
    // std::cout << "Unboxed message: " << unboxed;
    // std::cout << std::endl;

    // boxed = client_boxer.box("From sodiumpp!\n", used_n);
    // unboxed = server_unboxer.unbox(boxed, used_n);
    // std::cout << "Nonce (hex): " << used_n.get(encoding::hex).bytes << std::endl;
    // std::cout << "Boxed message (z85): " << boxed.to(encoding::z85).bytes << std::endl;
    // std::cout << "Unboxed message: " << unboxed;
    return 0;
}