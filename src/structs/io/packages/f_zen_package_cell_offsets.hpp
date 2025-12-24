//
// Created by oleub on 20.12.25.
//

#pragma once
#include <cstdint>

namespace SAP {
#pragma pack(push,1)
  struct FZenPackageCellOffsets {
    uint32_t CellImportMapOffset;
    uint32_t CellExportMapOffset;
  };
#pragma pack(pop)
}
