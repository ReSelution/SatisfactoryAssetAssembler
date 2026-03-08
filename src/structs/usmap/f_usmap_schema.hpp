//
// Created by oleub on 20.12.25.
//

#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>

#include "f_usmap_property_data.hpp"

namespace SAA {
  struct FUsmapSchema {
    std::string_view Name;
    std::string_view SuperName;
    std::pmr::unordered_map<uint16_t, FUsmapPropertyData> Properties; // Key: Property Index (nicht Name!)
  };

} // namespace SAP
