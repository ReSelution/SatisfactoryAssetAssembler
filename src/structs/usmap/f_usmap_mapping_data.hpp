//
// Created by oleub on 20.12.25.
//

#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#include "f_usmap_schema.hpp"
#include "version.hpp"

namespace SAA {
  struct FUsmapMappingData {
    FPackageFileVersion pkgVersion;
    std::vector<std::string_view> NameMap;
    std::unordered_map<std::string, std::vector<std::string_view>> Enums;
    std::unordered_map<std::string, FUsmapSchema> Schemas;
  };
} // namespace SAP
