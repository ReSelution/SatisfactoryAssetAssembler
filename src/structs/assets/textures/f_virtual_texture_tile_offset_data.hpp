//
// Created by oleub on 23.01.26.
//

#pragma once
#include <cstdint>
#include <vector>

namespace SAP {
  struct FVirtualTextureTileOffsetData {
    uint32_t width;
    uint32_t height;
    uint32_t maxAddress;
    std::span<const uint32_t> addresses;
    std::span<const uint32_t> offsets;
  };
}
