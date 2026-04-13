//
// Created by oleub on 07.02.26.
//

#pragma once
#include <cstdint>
#include <string_view>

namespace SAA {
  struct FTextKey {
    uint32_t strHash;
    std::string_view str;

    bool operator==(const FTextKey &other) const noexcept { return strHash == other.strHash; }
  };
} 
namespace std {
  template<>
  struct hash<SAA::FTextKey> {
    size_t operator()(const SAA::FTextKey &key) const noexcept { return static_cast<size_t>(key.strHash);; }
  };
} // namespace std
