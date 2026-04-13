//
// Created by oleub on 07.01.26.
//

#pragma once
#include <cstdint>
#include <vector>

#include "byte_cursor.hpp"

namespace SAA {
  struct FRawIndexBuffer {
    std::vector<uint32_t> indicesStorag;
    std::span<const uint32_t> indices;
  };

  struct FRawStaticIndexBuffer : FRawIndexBuffer {
    static FRawStaticIndexBuffer parse(ByteCursor &cursor) {
      FRawStaticIndexBuffer ret{};
      bool is32Bit = cursor.readBool();

      auto size = cursor.read<uint32_t>();
      auto count = cursor.read<uint32_t>();
      if (is32Bit) {
        ret.indices = cursor.readArray<uint32_t>(count / 4);
      } else [[likely]] {
        const uint32_t numIndices = count / 2;
        auto tmp = cursor.readArray<uint16_t>(numIndices);
        ret.indicesStorag.reserve(numIndices);
        for (uint32_t i = 0; i < numIndices; ++i) {
          ret.indicesStorag.emplace_back(tmp[i]);
        }
        ret.indices = ret.indicesStorag;
      }
      cursor += 4; // bShouldExpandTo32Bit;
      return ret;
    }
  };
} 
