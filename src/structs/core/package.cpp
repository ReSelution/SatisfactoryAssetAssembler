//
// Created by oleub on 20.12.25.
//
#include "package.hpp"

#include <cassert>

#include "city.h"
#include "functions.hpp"

namespace SAP {
  FPackageId FPackageId::fromName(const std::string& name)
  {
    std::string lower = to_lower(name);
    std::vector<uint8_t> utf16 = utf8_to_utf16le_ascii(lower);
    uint64_t hash = CityHash64(reinterpret_cast<const char*>(utf16.data()), utf16.size());
    assert(hash != ~0uL && "Package name hash collision with InvalidId");
    return FPackageId(hash);
  }
}
