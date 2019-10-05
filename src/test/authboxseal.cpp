#include <sodiumpp/sodiumpp.h>
#include <string>
#include <vector>
#include <iostream>


using namespace sodiumpp;

int main(int argc, const char ** argv) {
    // Uses predefined nonce type with 64-bit sequential counter 
    // and constant random bytes for the rest
    int N = 1;

    box_secret_key ppop;
    // This will be the client code
    std::vector<box_secret_key> mixers;
    for(int i = 0; i < N; i++){
        box_secret_key x;
        mixers.push_back(x);
        std::cout << i << std::endl;
    }
    
    std::string x = "Hello DEMS";
    for(int i = 0; i < N; i++){
        nonce64 tmp;
        //std::cout << "IN " << i << std::endl;
        sealedboxer<nonce64> mixer_boxer(mixers[i].pk);
        encoded_bytes boxed = mixer_boxer.box(x);
        x = boxed.bytes;
        std::cout << x << std::endl;
    }
    std::cout << "MADE IT HERE" << std::endl;
    // Now unbox this
    
    std::string unboxed;
    for(int i = (N-1); i > -1; i--){
        
        std::cout << i << std::endl;

        if(i == (N-1)){
            encoded_bytes j(x, encoding::binary);
            sealedunboxer<nonce64> mixer_unboxer(mixers[i].pk, mixers[i]); 
            std::cout << "MADE IT HERE" << std::endl;
            unboxed = mixer_unboxer.unbox(j);
            std::cout << "WORKS" << std::endl;
        }else{
            encoded_bytes v(unboxed, encoding::binary);
            sealedunboxer<nonce64> mixer_unboxer(mixers[i].pk, mixers[i]); 
            unboxed = mixer_unboxer.unbox(v);
            std::cout << "Unboxed message: " << i << "  " << unboxed << std::endl;
        } 

    }

    return 0;
}