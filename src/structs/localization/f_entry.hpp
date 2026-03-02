//
// Created by oleub on 07.02.26.
//

#pragma once
#include <cstdint>
#include <string_view>

namespace SAA {
  struct FEntry {
    std::string_view localizedString;
    uint32_t strHash;
  };
} // namespace SAP
