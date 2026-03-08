//
// Created by oleub on 20.12.25.
//

#pragma once
#include <string>
#include <unordered_map>

#include "f_usmap_schema.hpp"
#include "version.hpp"

#include "cache/chaos_slab.hpp"

namespace SAA {
  struct FUsmapMappingData {
    SC::ChaosSlab storage;
    FPackageFileVersion pkgVersion;
    std::span<std::string_view> NameMap;
    std::pmr::unordered_map<std::string_view, std::span<std::string_view>> Enums;
    std::pmr::unordered_map<std::string_view, FUsmapSchema> Schemas;
  };
} // namespace SAP
