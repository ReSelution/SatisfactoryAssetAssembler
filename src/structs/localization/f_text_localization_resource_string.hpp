//
// Created by oleub on 07.02.26.
//

#pragma once
#include <cstdint>
#include <string_view>

namespace SAA {
  struct FTextLocalizationResourceString {
    std::string_view str;
    int32_t refCount;
  };
} // namespace SAP
