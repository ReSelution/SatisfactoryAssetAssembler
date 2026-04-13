//
// Created by oleub on 20.12.25.
//

#pragma once
#include <cstdint>
#include <vector>

#include "f_io_hash.hpp"

namespace SAA {

#pragma pack(push, 1)

  struct FFilePackageStoreEntry {
    uint32_t exportCount;
    uint32_t exportBundleCount;
    std::span<const FPackageId> importedPackage;
    std::span<const FSHAHash> shaderMapHashes{};
  };

#pragma pack(pop)
} 
