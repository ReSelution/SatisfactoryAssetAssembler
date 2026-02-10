//
// Created by oleub on 09.01.26.
//

#pragma once
#include <cstdint>
#include <vector>

namespace SAP {
  struct FMeshUVChannelInfo {
    constexpr static uint32_t TEXSTREAM_MAX_NUM_UVCHANNELS =4;
    bool initialized;
    bool overrideDensities;
    std::span<const float> densities;

    static FMeshUVChannelInfo parse(ByteCursor &cursor) {
      return {
      .initialized = cursor.readBool(),
      .overrideDensities = cursor.readBool(),
      .densities = cursor.readArray<float>(TEXSTREAM_MAX_NUM_UVCHANNELS)
      };
    }
  };
}
