//
// Created by oleub on 20.12.25.
//

#pragma once
#include <cstdint>

namespace SAA {
  enum class EExportCommandType : uint32_t { Create, Serialize, Count };

#pragma pack(push, 1)
  struct FExportBundleEntry {
    uint32_t LocalExportIndex;
    EExportCommandType CommandType;
  };
#pragma pack(pop)
  static_assert(sizeof(FExportBundleEntry) == 8, "FExportBundleEntry size mismatch!");

} 
