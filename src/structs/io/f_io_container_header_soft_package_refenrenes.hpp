//
// Created by oleub on 20.12.25.
//

#pragma once
#include <cstdint>
#include <vector>

namespace SAA {
  struct FPackageId;
#pragma pack(push, 1)

  struct FIoContainerHeaderSoftPackageReferences {
    std::span<const FPackageId> PackageIds;
    std::span<const uint8_t> PackageIndices;
    bool bContainsSoftPackageReferences = false;
  };

#pragma pack(pop)
} // namespace SAP
