//
// Created by oleub on 20.12.25.
//
#include "package.hpp"

#include <cassert>

#include "city.h"
#include "functions.hpp"

namespace SAP {
  FPackageId FPackageId::fromName(const std::string_view name) {
    char16_t buffer[512];
    const size_t len = std::min(name.length(), (size_t) 511);

    for (size_t i = 0; i < len; ++i) {
      // Lowercase + UTF16-Promoting in einem Schritt
      const auto uc = static_cast<unsigned char>(name[i]);
      char lc = static_cast<char>(uc + (((uc - 'A' < 26u) ? 1u : 0u) << 5));
      buffer[i] = static_cast<char16_t>(lc);
    }

    uint64_t hash = CityHash64(reinterpret_cast<const char *>(buffer), len * sizeof(char16_t));
    assert(hash != ~0uL && "Package name hash collision with InvalidId");
    return FPackageId(hash);
  }
} // namespace SAP
