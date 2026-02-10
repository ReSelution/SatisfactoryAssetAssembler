//
// Created by oleub on 07.01.26.
//

#pragma once
#include <cstdint>
#include <vector>

#include "f_static_mesh_UV_item.hpp"
#include "../../../util/byte_cursor.hpp"
#include "ue/properties/f_property_tag.hpp"
#include <cassert>

namespace SAP {
  struct FStaticMeshVertexBuffer {
    int32_t numTextCoords{};
    int32_t numVertices{};
    std::vector<FStaticMeshUVItem> UVs;
    std::vector<FMeshUVFloat> uvStorage;

    static FStaticMeshVertexBuffer parse(ByteCursor &cursor) {
      auto stripDataFlags = cursor.read<FStripDataFlags>();
      FStaticMeshVertexBuffer result{
        .numTextCoords = cursor.read<int32_t>(),
        .numVertices = cursor.read<int32_t>(),
      };

      bool useFullPrecision = cursor.read<uint32_t>() != 0;
      bool useHighPrecisionTangentBasis = cursor.read<uint32_t>() != 0;
      assert(!useHighPrecisionTangentBasis && "useHighPrecisionTangentBasis not implented");
      auto &UVs = result.UVs;
      UVs.resize(result.numVertices);
      auto itemSize = cursor.read<int32_t>();
      auto itemCount = cursor.read<int32_t>();

      assert(itemCount == result.numVertices);

      auto tmp = cursor.readArray<uint32_t>(result.numVertices * 2);
      for (auto i = 0; i < result.numVertices; ++i) {
        UVs[i].normals[0].data = tmp[i * 2] ^ 0x80808080u;
        UVs[i].normals[2].data = tmp[i * 2 + 1] ^ 0x80808080u;
      }

      itemSize = cursor.read<int32_t>();
      itemCount = cursor.read<int32_t>();


      if (useFullPrecision) {
        result.uvStorage.resize(result.numVertices * result.numTextCoords);
        auto tmpUV = cursor.readArray<FMeshUVFloat>(result.numVertices * result.numTextCoords);
        result.uvStorage.assign(tmpUV.begin(), tmpUV.end()) ;
      }else {
        result.uvStorage.reserve(result.numVertices * result.numTextCoords);
        auto tmpUV = cursor.readArray<FMeshUVHalf>(result.numVertices * result.numTextCoords);
        for (auto uvHalf : tmpUV) {
          result.uvStorage.emplace_back(FMeshUVFloat::fromHalf(uvHalf));
        }
      }

      for (auto i = 0; i < result.numVertices; ++i) {
        std::span
            (
             result.uvStorage.data() + i * result.numTextCoords,
             result.numTextCoords
            );
      }

      return result;
    }
  };
}
