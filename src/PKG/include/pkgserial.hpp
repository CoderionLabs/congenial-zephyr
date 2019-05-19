#pragma once

#include <cereal/types/string.hpp>
#include <cereal/archives/binary.hpp>
#include <string>
#include <algorithm>
#include <stdexcept>
#include <boost/locale/conversion.hpp>

struct pkgkeys{
    std::string keys;
    std::string params;

    template<class Archive>
    void serialize(Archive & archive)
    {
        archive(keys, params);
    }
};
typedef struct pkgkeys pkgkeys;

std::string string_to_hex(const std::string& input);
std::string hex_to_string(const std::string& input);