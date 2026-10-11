//
// Created by oleub on 20.12.25.
//

#pragma once

#include <ankerl/unordered_dense.h>
#include <span>
#include "sc/hash/hash.hpp"

#include "f_usmap_schema.hpp"
#include "sc/memory/monotonic_resource.hpp"
#include "version.hpp"


namespace SAA {
  struct FUsmapMappingData {
    using denseMapEnums   = ankerl::unordered_dense::pmr::map<uint64_t, std::span<std::string_view>, sc::IdentityHash>;
    using denseMapSchemas = ankerl::unordered_dense::pmr::map<uint64_t, FUsmapSchema, sc::IdentityHash>;

    sc::MonotonicResource storage;
    FPackageFileVersion pkgVersion;
    std::span<std::string_view> NameMap;
    denseMapEnums Enums;
    denseMapSchemas Schemas;
  };
} // namespace SAA
