//
// Created by oleub on 20.12.25.
//

#pragma once

#include <string>
#include "hash/hash.hpp"
#include <ankerl/unordered_dense.h>

#include "f_usmap_schema.hpp"
#include "version.hpp"


namespace SAA {
    struct FUsmapMappingData {
        using denseMapEnums = ankerl::unordered_dense::pmr::map<uint64_t, std::span<std::string_view>, SC::IdentityHash>;
        using denseMapSchemas = ankerl::unordered_dense::pmr::map<uint64_t, FUsmapSchema, SC::IdentityHash>;

        SC::ChaosBumpArena storage;
        FPackageFileVersion pkgVersion;
        std::span<std::string_view> NameMap;
        denseMapEnums Enums;
        denseMapSchemas Schemas;
    };
} 
