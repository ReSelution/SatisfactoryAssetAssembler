//
// Created by oleub on 20.12.25.
//

#pragma once
#include <cstdint>
#include <vector>

#include "f_io_hash.hpp"

namespace SAP {


#pragma pack(push, 1)

  struct FFilePackageStoreEntry {
    uint32_t exportCount{};
    uint32_t exportBundleCount{};
    std::vector<FPackageId> importedPackage;
    std::vector<FIoHash> shaderMapHashes{};
  };

#pragma pack(pop)
}
