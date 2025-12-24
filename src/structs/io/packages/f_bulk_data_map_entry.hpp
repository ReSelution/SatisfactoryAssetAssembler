//
// Created by oleub on 20.12.25.
//

#pragma once
#include <cstdint>

#include "f_bulk_data_cooked_index.hpp"

namespace SAP {

  constexpr uint32_t FBulkDataMapEntrySize = 32;

#pragma pack(push, 1)
  struct FBulkDataMapEntry {
    uint64_t SerialOffset;
    uint64_t DuplicateSerialOffset;
    uint64_t SerialSize;
    uint32_t Flags;
    FBulkDataCookedIndex CookedIndex;
    int16_t _pad0;
    uint8_t _pad1;
  };
#pragma pack(pop)

  static_assert(sizeof(FBulkDataMapEntry) == FBulkDataMapEntrySize, "FBulkDataMapEntry size mismatch!");

}
