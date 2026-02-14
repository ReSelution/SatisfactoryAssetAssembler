//
// Created by oleub on 23.01.26.
//

#pragma once
#include <cstdint>
#include <vector>

#include "f_virtual_texture_tile_offset_data.hpp"
#include "f_virtual_texturte_data_chunk.hpp"
#include "pixel_format.hpp"

#include "core/f_linear_color.hpp"

namespace SAP {
  struct FVirtualTextureBuiltData {
    uint32_t numLayers;
    uint32_t numMips;
    uint32_t width;
    uint32_t height;
    uint32_t widthInBlocks;
    uint32_t heightInBlocks;
    uint32_t tileSize;
    uint32_t tileBorderSize;

    std::vector<EPixelFormat> layerTypes;
    std::vector<FVirtualTextureDataChunk> chunks;

    // Die optionalen Index- und Offset-Daten
    std::span<const uint32_t> tileIndexPerChunk;
    std::span<const uint32_t> tileIndexPerMip;
    std::span<const uint32_t> tileOffsetInChunk;
    std::span<const uint32_t> chunkIndexPerMip;
    std::span<const uint32_t> baseOffsetPerMip;
    std::span<const uint32_t> tileDataOffsetPerLayer;

    // Komplexe Sub-Strukturen
    std::vector<FVirtualTextureTileOffsetData> tileOffsetData;
    std::span<const FLinearColor> layerFallbackColors;
  };
} // namespace SAP
