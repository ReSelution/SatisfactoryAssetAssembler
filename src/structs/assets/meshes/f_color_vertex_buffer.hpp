//
// Created by oleub on 07.01.26.
//

#pragma once
#include <vector>

#include "core/f_color.hpp"

namespace SAP {
  struct FColorVertexBuffer {
    int32_t stride;
    int32_t numVertices;
    std::span<const FColor> data;

    static FColorVertexBuffer parse(ByteCursor &cursor) {
      auto stripDataFlags = cursor.read<FStripDataFlags>();
      FColorVertexBuffer ret{
        .stride  = cursor.read<int32_t>(),
        .numVertices = cursor.read<int32_t>(),
      };
      if (!stripDataFlags.isAudioVisualDataStripped() && ret.numVertices > 0) {
        ret.data = cursor.readBulkArray<FColor>();
      }
      return ret;
    }
  };
}
