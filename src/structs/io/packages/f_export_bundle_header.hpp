//
// Created by oleub on 20.12.25.
//

#pragma once
#include <cstdint>

namespace SAA {
#pragma pack(push, 1)
  struct FExportBundleHeader {
    uint64_t SerialOffset; // SerialOffset für UE5
    uint32_t FirstEntryIndex;
    uint32_t EntryCount;
  };

  static_assert(sizeof(FExportBundleHeader) == 16, "FExportBundleHeader size mismatch!");

#pragma pack(pop)
} // namespace SAP
