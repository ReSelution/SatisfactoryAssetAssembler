//
// Created by oleub on 07.01.26.
//

#pragma once
#include <cstdint>

namespace SAA {
#pragma pack(push, 1)

  struct FStaticMeshSection {
    int32_t MaterialIndex;
    int32_t FirstIndex;
    int32_t NumTriangles;
    int32_t MinVertexIndex;
    int32_t MaxVertexIndex;

    uint32_t bEnableCollision;
    uint32_t bCastShadow;
    uint32_t bForceOpaque;
    uint32_t bVisibleInRayTracing;
    uint32_t bAffectDistanceFieldLighting;
  };
#pragma pack(pop)
} // namespace SAP
