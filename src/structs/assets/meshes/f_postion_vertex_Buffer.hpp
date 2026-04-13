//
// Created by oleub on 07.01.26.
//

#pragma once
#include <cstdint>
#include <vector>
#include "byte_cursor.hpp"
#include "core/math/f_vector.hpp"

namespace SAA {

  struct FPositionVertexBuffer {
    int32_t numVertices;
    int32_t stride;
    std::span<const FVector3F> vertices;
    static FPositionVertexBuffer parse(ByteCursor &cursor) {
      FPositionVertexBuffer result;
      result.stride = cursor.read<int32_t>();
      result.numVertices = cursor.read<int32_t>();
      result.vertices = cursor.readBulkArray<FVector3F>();
      return result;
    }
  };
} 
