//
// Created by oleub on 31.12.25.
//

#pragma once
#include <cstdint>

#include "io/packages/f_byte_bulk_data.hpp"

namespace SAP {
  struct FTexture2DMipMap {
    FByteBulkData data;
    int32_t sizeX;
    int32_t sizeY;
    int32_t sizeZ;
  };
} // namespace SAP
