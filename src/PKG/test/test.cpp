/*
 * Copyright (c) 2019 Doku Enterprise
 * Author: Friedrich Doku
 * -----
 * Last Modified: Sunday March 3rd 2019 5:16:52 pm
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

    //Test key Serialization
    auto keystr = p.serialize_bytestring(key);
    byte_string_t keytmp;
    deserialize_bytestring(keystr,keytmp);

    // Test params Serialization
    auto paramstr = p.serialize_params();
    params_t paramtmp;
    deserialize_params(paramstr, paramtmp);

    auto j2 = pkg_decrypt(str, ad_key, p.params);
    auto j = pkg_decrypt(str, keytmp, paramtmp);
}