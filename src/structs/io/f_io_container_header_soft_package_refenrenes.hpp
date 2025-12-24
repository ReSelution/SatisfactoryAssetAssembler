//
// Created by oleub on 20.12.25.
//

#pragma once
#include <cstdint>
#include <vector>

namespace SAP {
  struct FPackageId;
#pragma pack(push, 1)

  struct FIoContainerHeaderSoftPackageReferences {
    std::vector<FPackageId> PackageIds;
    std::vector<uint8_t> PackageIndices;
    bool bContainsSoftPackageReferences = false;
  };

#pragma pack(pop)
}
