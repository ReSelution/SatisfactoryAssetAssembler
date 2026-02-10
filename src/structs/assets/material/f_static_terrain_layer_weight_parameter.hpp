//
// Created by oleub on 13.01.26.
//

#pragma once
#include <cstdint>

namespace SAP {
  struct FStaticTerrainLayerWeightParameter {
    uint32_t weightMapIndex;
    bool weightBasedBlend;
  };
}
