#pragma once

#include <cereal/types/string.hpp>
#include <cereal/archives/binary.hpp>

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