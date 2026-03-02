//
// Created by oleub on 07.01.26.
//

#pragma once
#include <cstdint>

#include "nanite_constants.hpp"

namespace SAA {
#pragma pack(push, 1)
  struct FPageStreamingState {
    uint32_t BulkOffset;
    uint32_t BulkSize;
    uint32_t PageSize;
    uint32_t DependenciesStart;
    uint16_t DependenciesNum;
    uint8_t MaxHierarchyDepth;
    NanitePageFlag Flags;
  };

#pragma pack(pop)
} // namespace SAP
