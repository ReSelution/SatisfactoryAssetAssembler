//
// Created by oleub on 22.04.26.
//

#pragma once
#include "f_texture_2d_mip_map.hpp"
#include "pixel_format.hpp"

namespace SAA {
#pragma pack(push, 1)
  struct FSharedImage {
    int32_t sizeX;
    int32_t sizeY;
    int32_t sizeZ;
    EPixelFormatByte format;
    uint8_t gammaSpace;
    FTexture2DMipMap mip;
  };
#pragma pack(pop)
}
