//
// Created by oleub on 13.01.26.
//

#pragma once
#include <cstdint>

namespace SAA {
  struct FStaticTerrainLayerWeightParameter {
    uint32_t weightMapIndex;
    bool weightBasedBlend;
  };
} // namespace SAP
