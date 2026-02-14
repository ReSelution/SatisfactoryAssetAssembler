//
// Created by oleub on 20.12.25.
//

#pragma once
#include <cstdint>

namespace SAP {
#pragma pack(push, 1)
  struct FZenPackageCellOffsets {
    int32_t CellImportMapOffset;
    int32_t CellExportMapOffset;
  };
#pragma pack(pop)
} // namespace SAP
