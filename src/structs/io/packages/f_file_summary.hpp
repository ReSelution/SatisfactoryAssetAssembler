//
// Created by oleub on 20.12.25.
//

#pragma once
#include <cstdint>

namespace SAA {
#pragma pack(push, 1)
  struct FFileSummary {
    uint32_t Tag;
    int32_t LegacyFileVersion;
    int32_t LegacyUE3Version;
    int32_t FileVersionUE4;
    int32_t FileVersionUE5;
    int32_t LicenseeVersion;
    int32_t NameCount;
    int32_t NameOffset;
    int32_t ImportCount;
    int32_t ImportOffset;
    int32_t ExportCount;
    int32_t ExportOffset;
  };

#pragma pack(pop)

} // namespace SAP
