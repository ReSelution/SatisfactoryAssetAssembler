//
// Created by oleub on 07.01.26.
//

#pragma once
#include "core/half.hpp"

namespace SAA {
#pragma pack(push, 1)
  struct FMeshUVHalf {
    half u;
    half v;
  };

  struct FMeshUVFloat {
    float u;
    float v;
    static FMeshUVFloat fromHalf(FMeshUVHalf &half) {
      return FMeshUVFloat{.u = half.u.toFloat(), .v = half.v.toFloat()};
    }
  };

#pragma pack(pop)
} // namespace SAP
