// Just a test to prove correctness
//#define MESSAGE ((const unsigned char *) "The")
//#define MESSAGE_LEN 3
//#define CIPHERTEXT_LEN (crypto_secretbox_MACBYTES + MESSAGE_LEN)

#include <iostream>
#include <cassert>
#include <zephyr/pkg.hpp>
extern "C"{
    #include <sibe/ibe.h>
    #include <sibe/ibe_progs.h>
}
CONF_CTX *cnfctx;
params_t params;

int main(){
    using namespace std;

    PKG p;
    p.setup("dokuenterprise");
    byte_string_t key, ad_key;
    auto str = pkg_encrypt("fried", p.params,"Everyone still has a ways to go.");
    p.extract("fried", key);
    p.extract("ethan", ad_key);
    auto j2 = pkg_decrypt(str, ad_key, p.params);
    auto j = pkg_decrypt(str, key, p.params);
    //byte_string_clear(key);
}