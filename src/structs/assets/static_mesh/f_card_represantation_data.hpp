//
// Created by oleub on 09.01.26.
//

#pragma once
#include "byte_cursor.hpp"

#include "core/math/f_box.hpp"

namespace SAA {
#pragma pack(push, 1)
  struct FLumenCardOBB {
    FVector_f axisX;
    FVector_f axisY;
    FVector_f axisZ;
    FVector_f origin;
    FVector_f extent;
  };

  struct FLumenCardBuildData {
    FLumenCardOBB obb;
    uint8_t axisAlignedDirectionIndex;
  };

#pragma pack(pop)
  struct FCardRepresentationData {
    FBox bounds;
    int32_t maxLODLevel;
    bool bMostlyTwoSided;
    std::span<const FLumenCardBuildData> buildData;

    static FCardRepresentationData parse(ByteCursor &cursor) {
      return {
          .bounds = cursor.read<FBox>(),
          .maxLODLevel = 0,
          .bMostlyTwoSided = cursor.readBool(),
          .buildData = cursor.readArray<FLumenCardBuildData>(),
      };
    }
  };
} // namespace SAP
