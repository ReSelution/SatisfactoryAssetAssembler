//
// Created by oleub on 20.12.25.
//

#pragma once
#include <array>
#include <cstdint>

#include "core/flags.hpp"
#include "core/name.hpp"
#include "core/package.hpp"

namespace SAA {
#pragma pack(push, 1)
  struct FExportMapEntry {
    uint64_t CookedSerialOffset;
    uint64_t CookedSerialSize;
    FMappedName ObjectName;
    FPackageObjectIndex OuterIndex;
    FPackageObjectIndex ClassIndex;
    FPackageObjectIndex SuperIndex;
    FPackageObjectIndex TemplateIndex;
    uint64_t PublicExportHash;
    EObjectFlags ObjectFlags;
    uint8_t FilterFlags;
    std::array<uint8_t, 3> padding;
  };

#pragma pack(pop)
} // namespace SAP
