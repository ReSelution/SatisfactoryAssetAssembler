//
// Created by oleub on 20.12.25.
//

#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#include "f_usmap_schema.hpp"
#include "version.hpp"

namespace SAP {
  struct FUsmapMappingData {
    FPackageFileVersion pkgVersion;
    std::vector<std::string> NameMap;
    std::unordered_map<std::string, std::vector<std::string>> Enums;
    std::unordered_map<std::string, FUsmapSchema> Schemas;
  };
}
