//
// Created by oleub on 12.04.26.
//
#include "hash.hpp"
#include "city.h"

uint64_t cityHash64_ue(const std::string_view str) {
    char16_t buffer[512];
    const size_t len = std::min(str.length(), (size_t) 511);

    for (size_t i = 0; i < len; ++i) {
        const uint8_t uc = static_cast<uint8_t>(str[i]);
        uint8_t lc = (uc >= 'A' && uc <= 'Z') ? (uc + 32) : uc;
        buffer[i] = static_cast<char16_t>(lc);
    }

    uint64_t hash = CityHash64(reinterpret_cast<const char *>(buffer), len * sizeof(char16_t));
    return hash;
}